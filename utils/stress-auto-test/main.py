import inspect
import logging
import math
import platform
import subprocess
import sys
import time
from copy import deepcopy

import config
from dut_connection import SerialConnection, TelnetConnection
from excel_exporter import export_test_result
from power_supply import UDP6720
from profile_resolver import resolve_task
from recorder import PowerRecorder, calculate_energy


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)


DUT_CONTROL_PARAM_KEYS = {
    "ping_timeout",
    "ping_interval",
    "ping_attempt_timeout",
    "connect_retries",
    "connect_retry_interval"
}


def _build_ping_command(ip, attempt_timeout):
    if platform.system().lower() == "windows":
        timeout_ms = max(1, int(float(attempt_timeout) * 1000))
        return ["ping", "-n", "1", "-w", str(timeout_ms), ip]

    timeout_seconds = max(1, int(math.ceil(float(attempt_timeout))))
    return ["ping", "-c", "1", "-W", str(timeout_seconds), ip]


def wait_for_ping(ip, check_interval=3, timeout=120, attempt_timeout=1):
    check_interval = max(0.0, float(check_interval))
    attempt_timeout = max(0.1, float(attempt_timeout))
    timeout = None if timeout is None else max(0.0, float(timeout))

    started_at = time.monotonic()
    command = _build_ping_command(ip, attempt_timeout)

    while True:
        if timeout is not None:
            elapsed = time.monotonic() - started_at
            if elapsed >= timeout:
                raise TimeoutError(
                    f"Timed out after {timeout:g} seconds waiting for ping from {ip}"
                )

        try:
            result = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
                timeout=attempt_timeout + 2.0
            )
            if b"ttl=" in result.stdout.lower():
                return
        except subprocess.TimeoutExpired:
            pass
        except Exception as exc:
            logging.debug(f"[Test] Ping attempt failed: {exc}")

        time.sleep(check_interval)


def _as_cmd_list(value):
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if isinstance(value, (list, tuple)):
        return list(value)
    raise TypeError(
        f"Command config must be str/list/tuple/None, got {type(value).__name__}"
    )


def _normalize_job_name(name):
    name = str(name).strip()
    if name == "待机模式":
        return "standby"
    return name.lower()


def _normalize_jobs(task):
    raw_jobs = task.get("jobs")
    if raw_jobs is None:
        raw_jobs = task.get("job", "cpu")

    if isinstance(raw_jobs, str):
        raw_jobs = [
            item.strip()
            for item in raw_jobs.split(",")
            if item.strip()
        ]
    elif isinstance(raw_jobs, (list, tuple)):
        raw_jobs = list(raw_jobs)
    else:
        raise TypeError("CURRENT_TASK['jobs'] must be a string, list, or tuple")

    jobs = []

    for item in raw_jobs:
        if isinstance(item, str):
            jobs.append({"name": _normalize_job_name(item)})
        elif isinstance(item, dict):
            job = dict(item)
            name = job.get("name", job.get("job"))
            if not name:
                raise ValueError(f"Job item is missing name/job: {item}")
            job["name"] = _normalize_job_name(name)
            jobs.append(job)
        else:
            raise TypeError(
                f"Job item must be str or dict, got {type(item).__name__}"
            )

    if not jobs:
        raise ValueError(
            "No jobs configured. Set CURRENT_TASK['jobs'] or CURRENT_TASK['job']."
        )

    return jobs


def _get_profile_start_cmd(platform_profile, job_name):
    start_cmd_config = platform_profile.get("start_cmd", [])

    if isinstance(start_cmd_config, dict):
        if job_name in start_cmd_config:
            return _as_cmd_list(start_cmd_config[job_name])
        if "default" in start_cmd_config:
            return _as_cmd_list(start_cmd_config["default"])
        raise KeyError(
            f"No start_cmd configured for job '{job_name}', and no default exists"
        )

    return _as_cmd_list(start_cmd_config)


def _format_regex(pattern, job_name):
    if not pattern:
        return None
    return str(pattern).replace("{job}", job_name)


def _build_job_config(job_entry, platform_profile, stress_config):
    job_name = job_entry["name"]

    base_profile = deepcopy(
        getattr(config, "JOB_PROFILES", {}).get(job_name, {})
    )
    job_config = {**base_profile, **job_entry}
    job_config["name"] = job_name

    if job_config.get("start_cmd") is None:
        job_config["start_cmd"] = _get_profile_start_cmd(
            platform_profile,
            job_name
        )
    else:
        job_config["start_cmd"] = _as_cmd_list(
            job_config.get("start_cmd")
        )

    mode = str(
        job_config.get("mode")
        or ("duration" if job_name == "standby" else "regex")
    ).lower()
    job_config["mode"] = mode

    if mode == "duration":
        job_config["duration"] = float(job_config.get("duration", 600))
    elif mode == "regex":
        job_config["start_regex"] = _format_regex(
            job_config.get(
                "start_regex",
                stress_config.get("start_regex")
            ),
            job_name
        )
        job_config["end_regex"] = _format_regex(
            job_config.get(
                "end_regex",
                stress_config.get("end_regex")
            ),
            job_name
        )

        if not job_config["start_regex"]:
            raise ValueError(f"Job '{job_name}' is missing start_regex")
        if not job_config["end_regex"]:
            raise ValueError(f"Job '{job_name}' is missing end_regex")
    else:
        raise ValueError(
            f"Unsupported job mode '{mode}' for job '{job_name}'"
        )

    return job_config


def _split_connection_params(connection_params):
    control_params = {
        key: value
        for key, value in connection_params.items()
        if key in DUT_CONTROL_PARAM_KEYS
    }
    client_params = {
        key: value
        for key, value in connection_params.items()
        if key not in DUT_CONTROL_PARAM_KEYS
    }
    return client_params, control_params


def _create_dut_connection(connection_type, connection_params):
    if connection_type == "TELNET":
        return TelnetConnection(**connection_params)

    if connection_type == "COM":
        return SerialConnection(**connection_params)

    raise ValueError(f"Unsupported DUT connection type: {connection_type}")


def _connect_dut_with_retry(connection_type, connection_params):
    client_params, control_params = _split_connection_params(connection_params)

    retries = max(1, int(control_params.get("connect_retries", 1)))
    retry_interval = max(
        0.0,
        float(control_params.get("connect_retry_interval", 3.0))
    )
    last_error = None

    for attempt in range(1, retries + 1):
        logging.info(
            f"[Test] Connecting to DUT... attempt {attempt}/{retries}"
        )

        try:
            dut = _create_dut_connection(connection_type, client_params)
            logging.info(
                f"[Test] DUT connection established on attempt {attempt}/{retries}."
            )
            return dut
        except (KeyError, TypeError, ValueError):
            raise
        except Exception as exc:
            last_error = exc

            if attempt >= retries:
                break

            logging.warning(
                f"[Test] DUT connection attempt {attempt}/{retries} failed: {exc}. "
                f"Retrying in {retry_interval:g} seconds..."
            )
            time.sleep(retry_interval)

    raise ConnectionError(
        f"Failed to connect to DUT after {retries} attempts: {last_error}"
    ) from last_error


def _wait_for_dut(connection_type, connection_params):
    if connection_type == "TELNET":
        wait_for_ping(
            connection_params["ip"],
            check_interval=connection_params.get("ping_interval", 3),
            timeout=connection_params.get("ping_timeout", 120),
            attempt_timeout=connection_params.get("ping_attempt_timeout", 1)
        )
        logging.info("[Test] DUT network is reachable.")
    else:
        logging.info("[Test] COM connection selected. Ping wait is skipped.")


def _filter_kwargs_for_callable(callable_obj, kwargs):
    signature = inspect.signature(callable_obj)
    parameters = signature.parameters

    if any(
        parameter.kind == inspect.Parameter.VAR_KEYWORD
        for parameter in parameters.values()
    ):
        return dict(kwargs), []

    allowed_keys = {
        name
        for name, parameter in parameters.items()
        if name != "self"
        and parameter.kind in {
            inspect.Parameter.POSITIONAL_OR_KEYWORD,
            inspect.Parameter.KEYWORD_ONLY
        }
    }

    filtered = {
        key: value
        for key, value in kwargs.items()
        if key in allowed_keys
    }
    ignored = sorted(
        key
        for key in kwargs.keys()
        if key not in allowed_keys
    )
    return filtered, ignored


def _create_power_supply(power_supply_config):
    filtered_config, ignored_keys = _filter_kwargs_for_callable(
        UDP6720.__init__,
        power_supply_config
    )

    if ignored_keys:
        logging.warning(
            f"[Test] Ignored unsupported UDP6720 config keys: {ignored_keys}"
        )

    return UDP6720(**filtered_config)


def _export_result(
    board_name,
    os_name,
    job_name,
    power_profile,
    data,
    exporter_config
):
    total_joules = calculate_energy(data)
    total_wh = total_joules / 3600.0
    avg_power = sum(record[3] for record in data) / len(data) if data else 0
    total_time = data[-1][0] if data else 0

    summary_info = {
        "板卡型号": board_name,
        "操作系统": os_name,
        "测试任务": job_name,
        "供电电压(V)": power_profile["voltage"],
        "限流保护(A)": power_profile["current"],
        "测试总耗时(s)": round(total_time, 2),
        "平均功率(W)": round(avg_power, 3),
        "总功耗(J)": round(total_joules, 3),
        "总功耗(Wh)": round(total_wh, 6)
    }

    try:
        if exporter_config.get("use_module"):
            out_file = export_test_result(
                board_name,
                os_name,
                job_name,
                data,
                summary_info,
                excel_path=exporter_config.get("output_path"),
                result_json=exporter_config.get("merge_result_json"),
                power_unit=exporter_config.get("power_unit", "J")
            )
            logging.info(f"[Test] Export completed: {out_file}")
        else:
            logging.warning("[Test] Built-in exporter is disabled.")
    except Exception as exc:
        logging.error(f"[Test] Export failed: {exc}")


def _run_one_job(runtime, stress_config, exporter_config, job_entry, index, total):
    board_name = runtime["board_name"]
    os_name = runtime["os_name"]
    power_profile = runtime["power_profile"]
    platform_profile = runtime["platform_profile"]
    connection_type = runtime["dut_conn_type"]
    connection_params = runtime["dut_conn_params"]
    power_supply_config = runtime["power_supply_config"]

    job_config = _build_job_config(
        job_entry,
        platform_profile,
        stress_config
    )
    job_name = job_config["name"]

    logging.info(
        f"[Test] Start job {index}/{total} | board={board_name} | "
        f"os={os_name} | job={job_name} | mode={job_config['mode']} | "
        f"connection={connection_type}"
    )

    power = None
    recorder = None
    dut = None
    recorder_started = False
    job_succeeded = False

    try:
        power = _create_power_supply(power_supply_config)
        recorder = PowerRecorder(power)

        power.connect()
        power.set_protect_voltage(power_profile["protect_voltage"])
        power.set_protect_current(power_profile["protect_current"])
        power.set_voltage(power_profile["voltage"])
        power.set_current(power_profile["current"])
        power.turn_on()

        if board_name == "LS2K1000LA":
            logging.info(
                "[Test] Press the board power button if manual startup is required."
            )

        _wait_for_dut(connection_type, connection_params)
        dut = _connect_dut_with_retry(connection_type, connection_params)

        start_cmd = job_config.get("start_cmd", [])
        if start_cmd:
            logging.info(
                f"[Test] Sending start_cmd exactly as configured: {start_cmd}"
            )
            dut.send_cmd(start_cmd)
        else:
            logging.info("[Test] start_cmd is empty. No DUT command will be sent.")

        if job_config["mode"] == "duration":
            duration = job_config["duration"]
            logging.info(
                f"[Test] Duration mode recording for {duration:g} seconds..."
            )
            recorder.start()
            recorder_started = True
            time.sleep(duration)
            logging.info("[Test] Duration mode completed.")

        elif job_config["mode"] == "regex":
            logging.info(
                f"[Test] Waiting for start pattern: {job_config['start_regex']}"
            )
            dut.wait_for_regex(job_config["start_regex"])

            recorder.start()
            recorder_started = True

            logging.info(
                f"[Test] Waiting for end pattern: {job_config['end_regex']}"
            )
            dut.wait_for_regex(job_config["end_regex"])
            logging.info("[Test] End pattern matched.")

        data = recorder.stop() if recorder_started else []
        recorder_started = False

        _export_result(
            board_name,
            os_name,
            job_name,
            power_profile,
            data,
            exporter_config
        )
        job_succeeded = True

    except Exception as exc:
        logging.error(f"[Test] Job '{job_name}' failed: {exc}")

        if recorder_started and recorder:
            try:
                data = recorder.stop()
                recorder_started = False
                _export_result(
                    board_name,
                    os_name,
                    job_name,
                    power_profile,
                    data,
                    exporter_config
                )
            except Exception as stop_exc:
                logging.error(
                    "[Test] Failed to stop recorder or export partial data: "
                    f"{stop_exc}"
                )

    finally:
        if dut:
            try:
                time.sleep(5)
                shutdown_cmd = _as_cmd_list(
                    platform_profile.get("shutdown_cmd")
                )

                if shutdown_cmd:
                    logging.info("[Test] Sending shutdown command...")
                    dut.send_cmd(shutdown_cmd)
                    time.sleep(5)
            except Exception as exc:
                logging.error(f"[Test] Failed to send shutdown command: {exc}")

        if power:
            try:
                power.turn_off()
            except Exception:
                pass

            try:
                power.disconnect()
            except Exception:
                pass

        if dut:
            try:
                dut.disconnect()
            except Exception:
                pass

        status = "succeeded" if job_succeeded else "failed"
        logging.info(
            f"[Test] Job '{job_name}' finished with status={status}. "
            "DUT disconnected."
        )

    return job_succeeded


def run_automation():
    runtime = resolve_task(config.CURRENT_TASK)
    jobs = _normalize_jobs(runtime["task"])
    stress_config = config.STRESS_CONFIG
    exporter_config = config.EXCEL_EXPORTER
    stop_on_job_failure = bool(
        runtime["task"].get("stop_on_job_failure", True)
    )
    failed_jobs = []

    logging.info(
        f"[Test] Job queue started | board={runtime['board_name']} | "
        f"os={runtime['os_name']} | connection={runtime['dut_conn_type']} | "
        f"jobs={[job['name'] for job in jobs]}"
    )

    for index, job_entry in enumerate(jobs, start=1):
        succeeded = _run_one_job(
            runtime,
            stress_config,
            exporter_config,
            job_entry,
            index,
            len(jobs)
        )

        if not succeeded:
            failed_jobs.append(job_entry["name"])

            if stop_on_job_failure:
                logging.error(
                    f"[Test] Job queue aborted after failed job "
                    f"'{job_entry['name']}'."
                )
                break

    if failed_jobs:
        logging.error(
            f"[Test] Job queue finished with failed jobs: {failed_jobs}"
        )
        return False

    logging.info("[Test] All jobs completed successfully.")
    return True


if __name__ == "__main__":
    sys.exit(0 if run_automation() else 1)
