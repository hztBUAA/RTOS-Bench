import json

INPUT_FILE = "standard_results_example.json"
OUTPUT_FILE = "score_report.json"

A = 0.25


# -------------------------------------------------
# 需要的指标定义
# -------------------------------------------------

REQUIRED_METRICS = {

    "basic": [
        "mem_bandwidth",
        "mem_latency",
        "fs_small_iops",
        "fs_large_iops",
        "net_bandwidth",
        "net_latency",
        "posix_supported",
        "posix_total"
    ],

    "typical": [
        "ctrl_latency",
        "perception_latency",
        "comm_latency",
        "ops_latency"
    ],

    "realtime": [
        "soft_rt_score",
        "hard_rt_score",
        "miss_rate"
    ],

    "power": [
        "idle_power",
        "cpu_power",
        "mem_power",
        "fs_power"
    ]
}


# -------------------------------------------------
# JSON解析
# -------------------------------------------------

def parse_rtbench_json(path):

    with open(path) as f:
        raw = json.load(f)

    metrics = {}

    for item in raw:

        case = item["test_case_info"]["test_case"]
        value = item["test_result_info"]["test_result"]

        try:
            value = float(value)
        except:
            continue

        metrics[case] = value

    return metrics


# -------------------------------------------------
# 检查缺失数据
# -------------------------------------------------

def check_missing(metrics):

    missing = []

    for group in REQUIRED_METRICS.values():
        for m in group:
            if m not in metrics:
                missing.append(m)

    return missing


# -------------------------------------------------
# 工具函数
# -------------------------------------------------

def ratio(real, base, mode):

    if mode == "higher":
        return real / base

    else:
        return base / real


def load_score(real, base):

    R = base / real
    return R / (A + R)


# -------------------------------------------------
# 基础功能
# -------------------------------------------------

def calc_F(metrics, base):

    mem_bw = ratio(metrics["mem_bandwidth"], base["mem_bandwidth"], "higher")
    mem_lat = ratio(metrics["mem_latency"], base["mem_latency"], "lower")

    small_iops = ratio(metrics["fs_small_iops"], base["fs_small_iops"], "higher")
    large_iops = ratio(metrics["fs_large_iops"], base["fs_large_iops"], "higher")

    net_bw = ratio(metrics["net_bandwidth"], base["net_bandwidth"], "higher")
    net_lat = ratio(metrics["net_latency"], base["net_latency"], "lower")

    posix = metrics["posix_supported"] / metrics["posix_total"]

    score = (
                    mem_bw * 0.15 +
                    mem_lat * 0.15 +
                    small_iops * 0.125 +
                    large_iops * 0.125 +
                    net_bw * 0.125 +
                    net_lat * 0.125 +
                    posix * 0.25
            ) * 100

    return score


# -------------------------------------------------
# 典型负载
# -------------------------------------------------

def calc_T(metrics, base):

    ctrl = load_score(metrics["ctrl_latency"], base["ctrl_latency"])
    perception = load_score(metrics["perception_latency"], base["perception_latency"])
    comm = load_score(metrics["comm_latency"], base["comm_latency"])
    ops = load_score(metrics["ops_latency"], base["ops_latency"])

    return (
            ctrl * 0.25 +
            perception * 0.25 +
            comm * 0.25 +
            ops * 0.25
    )


# -------------------------------------------------
# 实时性能
# -------------------------------------------------

def calc_RT(metrics):

    soft = metrics["soft_rt_score"]
    hard = metrics["hard_rt_score"]

    miss = metrics["miss_rate"]

    sched = 100 * (1 - miss)

    return soft * 0.3 + hard * 0.3 + sched * 0.4


# -------------------------------------------------
# 功耗
# -------------------------------------------------

def calc_P(metrics, base, min_ratio, max_ratio):

    ratio_val = (
                        base["idle_power"] / metrics["idle_power"] +
                        base["cpu_power"] / metrics["cpu_power"] +
                        base["mem_power"] / metrics["mem_power"] +
                        base["fs_power"] / metrics["fs_power"]
                ) / 4

    return 100 * (ratio_val - min_ratio) / (max_ratio - min_ratio)


# -------------------------------------------------
# 总分
# -------------------------------------------------

def calc_total(F, RT, T, P):

    return F * 0.2 + RT * 0.3 + T * 0.4 + P * 0.1


# -------------------------------------------------
# 主程序
# -------------------------------------------------

def main():

    metrics = parse_rtbench_json(INPUT_FILE)

    missing = check_missing(metrics)

    report = {
        "missing_metrics": missing
    }

    if missing:
        report["status"] = "missing_data"
    else:

        with open("baseline.json") as f:
            base = json.load(f)

        F = calc_F(metrics, base["basic"])
        T = calc_T(metrics, base["typical"])
        RT = calc_RT(metrics)
        P = calc_P(metrics, base["power"], base["power_min"], base["power_max"])

        total = calc_total(F, RT, T, P)

        report.update({
            "F_score": F,
            "RT_score": RT,
            "T_score": T,
            "P_score": P,
            "Total_score": total
        })

    with open(OUTPUT_FILE, "w") as f:
        json.dump(report, f, indent=2)


if __name__ == "__main__":
    main()
