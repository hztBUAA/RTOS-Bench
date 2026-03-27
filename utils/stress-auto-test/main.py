import config
from power_supply import UDP6720
from dut_connection import SSHConnection, SerialConnection, TelnetConnection
from recorder import PowerRecorder, calculate_energy
from excel_exporter import export_test_result
import time
import subprocess
import platform
import logging

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S")

def wait_for_ping(ip, check_interval=3):
    param = '-n' if platform.system().lower() == 'windows' else '-c'
    command = ['ping', param, '1', ip]
    while True:
        try:
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if b"ttl=" in result.stdout.lower():
                break
        except Exception:
            pass
        time.sleep(check_interval)

def run_automation():
    task = config.CURRENT_TASK
    board_name = task["board"]
    os_name = task["os"]
    job_type = task.get("job", "cpu")
    is_debug = task.get("is_debug", False)
    board_cfg = config.BOARD_PROFILES[board_name]
    os_cfg = config.OS_PROFILES[os_name]
    stress_cfg = config.STRESS_CONFIG
    exporter_cfg = config.EXCEL_EXPORTER

    logging.info(f"[Test]启动测试 | 板卡: {board_name} | 系统: {os_name} | 任务: {job_type}")

    power = UDP6720(port=task["power_port"], baudrate=9600)
    recorder = PowerRecorder(power)
    dut = None

    try:
        power.connect()
        power.set_protect_voltage(board_cfg["protect_voltage"])
        power.set_protect_current(board_cfg["protect_current"])
        power.set_voltage(board_cfg["voltage"])
        power.set_current(board_cfg["current"])

        power.turn_on()

        logging.info("[Test]请手动按下龙芯派上的[电源按钮]启动设备。正在等待网络连通...")
        wait_for_ping(task["dut_conn_params"]["ip"])
        logging.info("[Test]设备网络已就绪，正在建立连接...")

        if task["dut_conn_type"] == "SSH":
            dut = SSHConnection(**task["dut_conn_params"])
        elif task["dut_conn_type"] == "TELNET":
            dut = TelnetConnection(**task["dut_conn_params"])
        else:
            dut = SerialConnection(**task["dut_conn_params"])

        if job_type.lower() == "standby" or job_type == "待机模式":
            logging.info("[Test]进入待机模式，开始录制10分钟...")
            recorder.start()
            time.sleep(600)
            logging.info("[Test]待机结束，停止录制。")
        else:
            logging.info("[Test]发送测试启动指令...")
            cmds = list(os_cfg["start_cmd"])
            target_cmd = f"{cmds[-1]} --job {job_type.lower()}"
            if is_debug:
                target_cmd += " --debug"
            cmds[-1] = target_cmd

            dut.send_cmd(cmds)

            logging.info("[Test]等待测试开始信号...")
            dut.wait_for_regex(stress_cfg["start_regex"])
            recorder.start()

            dut.wait_for_regex(stress_cfg["end_regex"])
            logging.info("[Test]监听到结束信号，停止录制。")

        data = recorder.stop()
        total_joules = calculate_energy(data)
        total_wh = total_joules / 3600.0

        avg_power = sum([r[3] for r in data]) / len(data) if data else 0
        total_time = data[-1][0] if data else 0

        summary_info = {
            "板卡型号": board_name,
            "操作系统": os_name,
            "测试任务": job_type,
            "供电电压(V)": board_cfg["voltage"],
            "限流保护(A)": board_cfg["current"],
            "测试总耗时(s)": round(total_time, 2),
            "平均功率(W)": round(avg_power, 3),
            "总功耗(J)": round(total_joules, 3),
            "总功耗(Wh)": round(total_wh, 6)
        }

        # 调用导出器，使用 config 中的设置
        try:
            if exporter_cfg.get("use_module"):
                excel_path = exporter_cfg.get("output_path")
                merge_json = exporter_cfg.get("merge_result_json")
                power_unit = exporter_cfg.get("power_unit", "J")
                out_file = export_test_result(board_name, os_name, job_type, data, summary_info, excel_path=excel_path, result_json=merge_json, power_unit=power_unit)
                logging.info(f"[Test] 导出完成: {out_file}")
            else:
                # TODO: support calling external cmd template if needed
                logging.warning("[Test] 未启用内置 exporter 模块。请配置 EXCEL_EXPORTER.cmd")
        except Exception as e:
            logging.error(f"[Test] 导出失败: {e}")

    except Exception as e:
        logging.error(f"[Test]异常: {e}")

    finally:
        if dut:
            try:
                time.sleep(5)
                logging.info("[Test] 正在执行系统安全关机...")
                if "shutdown_cmd" in os_cfg:
                    dut.send_cmd(os_cfg["shutdown_cmd"])
                    time.sleep(5)
            except Exception as e:
                logging.error(f"[Test] 安全关机指令发送失败: {e}")
        try:
            power.turn_off()
        except:
            pass

        try:
            power.disconnect()
        except:
            pass
        try:
            if dut:
                dut.disconnect()
        except:
            pass
        logging.info("[Test]测试结束，设备已断开连接。")


if __name__ == "__main__":
    run_automation()
