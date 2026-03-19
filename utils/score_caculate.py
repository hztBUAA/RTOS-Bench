"""
rtos_scorer.py
工业 RTOS 评测结果汇总与评分程序（min-max 归一化版本）

T_Score / RT_Score 子项得分公式：
  Score = (R - Rmin) / (Rmax - Rmin) * 100
  其中 Rmin / Rmax 来自同板卡所有 OS 对同一子项的 R 值集合

依赖: pandas, openpyxl
安装: pip install pandas openpyxl
"""

import json
import sqlite3
import logging
import os
import glob
from datetime import datetime, timezone
from typing import Optional
import warnings

import pandas as pd

warnings.filterwarnings("ignore", category=UserWarning, module="openpyxl")

# ─────────────────────────────────────────────
# 0. 全局配置
# ─────────────────────────────────────────────
CONFIG = {
    "default_function_score": 60.0,
    "workload_partial_mode":  True,
    "power_partial_mode":     True,
    "storage_path":           "./data/results_db.sqlite",
    "baseline_os":            "RT-Thread",
}

WORKLOAD_CATEGORY_MAP = {
    "pid":    "realtime_control",
    "ekf":    "perception",
    "icp":    "perception",
    "fast":   "perception",
    "epnp":   "perception",
    "modbus": "communication",
    "mqtt":   "communication",
    "cusum":  "monitoring",
    "ewma":   "monitoring",
}

WORKLOAD_CATEGORY_WEIGHTS = {
    "realtime_control": 0.25,
    "perception":       0.25,
    "communication":    0.25,
    "monitoring":       0.25,
}

BASE_RT_WEIGHTS = {
    "context_switch_latency": 0.10,
    "interrupt_latency":      0.10,
    "scheduling_latency":     0.40,
    "multicore_overhead":     0.40,
}

POWER_TASK_MAP = {
    "standby": "static_power",
    "cpu":     "cpu_power",
    "memory":  "memory_power",
    "file":    "file_power",
}

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("rtos_scorer")


# ─────────────────────────────────────────────
# 1. 数据库初始化
# ─────────────────────────────────────────────
def init_db(db_path: str) -> sqlite3.Connection:
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()

    # 原始子项表（存储 R_value，item_score 由归一化后写入）
    cur.execute("""
    CREATE TABLE IF NOT EXISTS raw_records (
        board_model      TEXT NOT NULL,
        os_name          TEXT NOT NULL,
        arch             TEXT NOT NULL,
        category         TEXT NOT NULL,
        sub_category     TEXT,
        item_key         TEXT NOT NULL,
        raw_value        REAL,
        raw_unit         TEXT,
        metric_type      TEXT,
        baseline_value   REAL,
        baseline_source  TEXT,
        R_value          REAL,
        item_score       REAL,           -- 归一化后写入
        norm_pending     INTEGER DEFAULT 1,  -- 1=待归一化
        status           TEXT,
        source_file      TEXT,
        last_updated     TEXT,
        PRIMARY KEY (board_model, os_name, arch, category, sub_category, item_key)
    )
    """)

    cur.execute("""
    CREATE TABLE IF NOT EXISTS score_cache (
        board_model  TEXT NOT NULL,
        os_name      TEXT NOT NULL,
        arch         TEXT NOT NULL,
        score_level  TEXT NOT NULL,
        score_key    TEXT NOT NULL,
        score_value  REAL,
        is_partial   INTEGER DEFAULT 0,
        extra_json   TEXT,
        last_updated TEXT,
        PRIMARY KEY (board_model, os_name, arch, score_level, score_key)
    )
    """)

    conn.commit()
    return conn


# ─────────────────────────────────────────────
# 2. 工具函数
# ─────────────────────────────────────────────
def safe_float(val) -> Optional[float]:
    try:
        f = float(val)
        return f if f == f else None
    except (TypeError, ValueError):
        return None


def calc_R(measured: float, baseline: float, metric_type: str) -> Optional[float]:
    """
    latency  → R = baseline / measured  (延迟越低越好，R 越大越好)
    bandwidth/ops → R = measured / baseline  (吞吐越大越好，R 越大越好)
    """
    if measured is None or baseline is None:
        return None
    if measured == 0 or baseline == 0:
        return None
    if metric_type == "latency":
        return baseline / measured
    else:
        return measured / baseline


def minmax_score(R: Optional[float],
                 R_min: float, R_max: float) -> Optional[float]:
    """
    Score = (R - Rmin) / (Rmax - Rmin) * 100
    R_min == R_max 时无法归一化，返回 None。
    """
    if R is None:
        return None
    if R_max == R_min:
        return None
    score = (R - R_min) / (R_max - R_min) * 100.0
    # 裁剪到 [0, 100]
    return max(0.0, min(100.0, score))


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def upsert_record(conn: sqlite3.Connection, rec: dict):
    conn.execute("""
    INSERT INTO raw_records
        (board_model, os_name, arch, category, sub_category, item_key,
         raw_value, raw_unit, metric_type,
         baseline_value, baseline_source, R_value, item_score,
         norm_pending, status, source_file, last_updated)
    VALUES
        (:board_model, :os_name, :arch, :category, :sub_category, :item_key,
         :raw_value, :raw_unit, :metric_type,
         :baseline_value, :baseline_source, :R_value, :item_score,
         :norm_pending, :status, :source_file, :last_updated)
    ON CONFLICT(board_model, os_name, arch, category, sub_category, item_key)
    DO UPDATE SET
        raw_value       = excluded.raw_value,
        raw_unit        = excluded.raw_unit,
        metric_type     = excluded.metric_type,
        baseline_value  = excluded.baseline_value,
        baseline_source = excluded.baseline_source,
        R_value         = excluded.R_value,
        item_score      = excluded.item_score,
        norm_pending    = excluded.norm_pending,
        status          = excluded.status,
        source_file     = excluded.source_file,
        last_updated    = excluded.last_updated
    """, rec)


def upsert_score(conn, board_model, os_name, arch,
                 score_level, score_key,
                 score_value: Optional[float],
                 is_partial: bool = False,
                 extra: dict = None):
    conn.execute("""
    INSERT INTO score_cache
        (board_model, os_name, arch, score_level, score_key,
         score_value, is_partial, extra_json, last_updated)
    VALUES (?,?,?,?,?,?,?,?,?)
    ON CONFLICT(board_model, os_name, arch, score_level, score_key)
    DO UPDATE SET
        score_value  = excluded.score_value,
        is_partial   = excluded.is_partial,
        extra_json   = excluded.extra_json,
        last_updated = excluded.last_updated
    """, (board_model, os_name, arch, score_level, score_key,
          score_value, int(is_partial),
          json.dumps(extra or {}),
          now_iso()))


def get_baseline_value(conn, board_model, arch,
                       category, sub_category, item_key) -> Optional[float]:
    row = conn.execute("""
    SELECT raw_value FROM raw_records
    WHERE board_model=? AND arch=? AND os_name=?
      AND category=? AND sub_category=? AND item_key=?
    """, (board_model, arch, CONFIG["baseline_os"],
          category, sub_category, item_key)).fetchone()
    return row["raw_value"] if row else None


# ─────────────────────────────────────────────
# 3. min-max 归一化（核心新增逻辑）
# ─────────────────────────────────────────────
def normalize_items_for_board(board_model: str, arch: str,
                               conn: sqlite3.Connection,
                               categories: tuple = ("realtime", "workload")):
    """
    对指定板卡+架构，在所有 OS 的同一子项 R_value 中做 min-max 归一化，
    将归一化后的 item_score 写回 raw_records。

    同时返回受影响的 (board_model, os_name, arch) 三元组集合，
    供后续重算汇总分数使用。

    注意：
    - schedulability 子项（metric_type='miss_rate'）直接用 Sched_Score=100*(1-MR)，
      不走 min-max，这里跳过。
    - power 子项另有归一化逻辑，不在此处理。
    """
    affected_os = set()

    # 获取该板卡下所有需要归一化的 (category, sub_category, item_key) 组合
    groups = conn.execute("""
    SELECT DISTINCT category, sub_category, item_key
    FROM raw_records
    WHERE board_model=? AND arch=?
      AND category IN ({})
      AND metric_type != 'miss_rate'
      AND R_value IS NOT NULL
    """.format(",".join("?" * len(categories))),
    (board_model, arch, *categories)).fetchall()

    for grp in groups:
        cat  = grp["category"]
        sub  = grp["sub_category"]
        key  = grp["item_key"]

        # 收集该板卡所有 OS 在这个子项上的 R_value
        rows = conn.execute("""
        SELECT os_name, R_value FROM raw_records
        WHERE board_model=? AND arch=?
          AND category=? AND sub_category=? AND item_key=?
          AND R_value IS NOT NULL
          AND status NOT IN ('null_value','error')
        """, (board_model, arch, cat, sub, key)).fetchall()

        if not rows:
            continue

        r_vals = [r["R_value"] for r in rows]
        R_min  = min(r_vals)
        R_max  = max(r_vals)

        for row in rows:
            os_name = row["os_name"]
            R       = row["R_value"]

            if R_max == R_min:
                # 所有系统表现相同，无法区分，标记 pending
                score        = None
                norm_pending = 1
            else:
                score        = minmax_score(R, R_min, R_max)
                norm_pending = 0

            conn.execute("""
            UPDATE raw_records
            SET item_score=?, norm_pending=?, last_updated=?
            WHERE board_model=? AND os_name=? AND arch=?
              AND category=? AND sub_category=? AND item_key=?
            """, (score, norm_pending, now_iso(),
                  board_model, os_name, arch, cat, sub, key))

            affected_os.add((board_model, os_name, arch))

    conn.commit()
    logger.info(
        f"归一化完成 [{board_model}|{arch}]: "
        f"{len(groups)} 个子项组，影响 {len(affected_os)} 个系统"
    )
    return affected_os


# ─────────────────────────────────────────────
# 4. JSON 解析（只存 R_value，不算 item_score）
# ─────────────────────────────────────────────
def parse_json_results(filepath: str, conn: sqlite3.Connection) -> dict:
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            data = json.load(f)
    except Exception as e:
        logger.error(f"JSON 解析失败 [{filepath}]: {e}")
        return {}

    env         = data.get("env", {})
    board_model = env.get("board", "unknown")
    os_name     = env.get("os_name", "unknown")
    arch        = env.get("cpu_type", "unknown")
    source      = os.path.basename(filepath)
    ts          = now_iso()

    logger.info(f"解析 JSON: board={board_model}, os={os_name}, arch={arch}")

    modules = data.get("modules", {})
    _parse_realtime_module(conn, modules, board_model, os_name, arch, source, ts)
    _parse_schedule_module(conn, modules, board_model, os_name, arch, source, ts)
    _parse_workload_module(conn, modules, board_model, os_name, arch, source, ts)
    conn.commit()

    return {"board_model": board_model, "os_name": os_name, "arch": arch}


def _make_rec_base(board_model, os_name, arch, category,
                   sub_category, item_key, source, ts) -> dict:
    return {
        "board_model": board_model, "os_name": os_name, "arch": arch,
        "category": category, "sub_category": sub_category, "item_key": item_key,
        "raw_value": None, "raw_unit": "us", "metric_type": "latency",
        "baseline_value": None, "baseline_source": None,
        "R_value": None,
        "item_score": None,    # 等待 normalize_items_for_board() 填入
        "norm_pending": 1,
        "status": "ok", "source_file": source, "last_updated": ts,
    }


def _fill_R_only(rec: dict, conn: sqlite3.Connection) -> dict:
    """
    只计算并存储 R_value，不计算 item_score。
    item_score 由 normalize_items_for_board() 统一归一化后写入。
    """
    if rec["raw_value"] is None:
        rec["status"] = "null_value"
        return rec

    if rec["baseline_value"] is None:
        bv = get_baseline_value(
            conn,
            rec["board_model"], rec["arch"],
            rec["category"], rec["sub_category"], rec["item_key"]
        )
        if bv is not None:
            rec["baseline_value"] = bv
            rec["baseline_source"] = "db"
        else:
            rec["status"] = "baseline_missing"
            return rec

    R = calc_R(rec["raw_value"], rec["baseline_value"], rec["metric_type"])
    rec["R_value"] = R
    rec["status"]  = "ok" if R is not None else "null_value"
    return rec


def _parse_realtime_module(conn, modules, board_model, os_name, arch, source, ts):
    mod = modules.get("test-realtime", {})
    if not mod:
        logger.warning("test-realtime 模块缺失")
        return

    sc = mod.get("single_core", {})

    # context_switch_latency
    ctx = sc.get("context_switch", {})
    if ctx:
        rec = _make_rec_base(board_model, os_name, arch,
                             "realtime", "context_switch_latency",
                             "avg_us", source, ts)
        rec["raw_value"] = safe_float(ctx.get("avg_us"))
        rec = _fill_R_only(rec, conn)
        upsert_record(conn, rec)

    # interrupt_latency
    intr = sc.get("interrupt", {})
    if intr:
        rec = _make_rec_base(board_model, os_name, arch,
                             "realtime", "interrupt_latency",
                             "avg_us", source, ts)
        rec["raw_value"] = safe_float(intr.get("avg_us"))
        if rec["raw_value"] == 0.0:
            rec["status"] = "null_value"
        else:
            rec = _fill_R_only(rec, conn)
        upsert_record(conn, rec)

    # scheduling_latency（service_cost 各场景）
    for entry in sc.get("service_cost", []):
        op = entry.get("operation", "unknown")
        for scenario in ["immediate_us", "suspend_us", "low_prio_us", "high_prio_us"]:
            val = safe_float(entry.get(scenario))
            if val is None or val == 0.0:
                continue
            item_key = f"{op}_{scenario}"
            rec = _make_rec_base(board_model, os_name, arch,
                                 "realtime", "scheduling_latency",
                                 item_key, source, ts)
            rec["raw_value"] = val
            rec = _fill_R_only(rec, conn)
            upsert_record(conn, rec)

    # syscall 归入 scheduling_latency
    syscall = sc.get("syscall", {})
    if syscall:
        rec = _make_rec_base(board_model, os_name, arch,
                             "realtime", "scheduling_latency",
                             "syscall_avg_us", source, ts)
        rec["raw_value"] = safe_float(syscall.get("avg_us"))
        rec = _fill_R_only(rec, conn)
        upsert_record(conn, rec)


def _parse_schedule_module(conn, modules, board_model, os_name, arch, source, ts):
    """
    schedulability 直接用 Sched_Score = 100*(1-avg_mr)，
    不走 min-max，在这里直接写入 item_score，norm_pending=0。
    """
    mod = modules.get("test-schedule", {})
    if not mod:
        logger.warning("test-schedule 模块缺失")
        return

    summary = mod.get("summary", {})
    avg_mr  = safe_float(summary.get("average_miss_rate"))

    rec = _make_rec_base(board_model, os_name, arch,
                         "realtime", "schedulability",
                         "average_mr", source, ts)
    rec["raw_value"]   = avg_mr
    rec["raw_unit"]    = "ratio"
    rec["metric_type"] = "miss_rate"
    rec["norm_pending"] = 0   # 不需要 min-max
    if avg_mr is not None:
        rec["item_score"] = 100.0 * (1.0 - avg_mr)
        rec["status"] = "ok"
    else:
        rec["status"] = "null_value"
    upsert_record(conn, rec)

    # 各梯度详情
    for grad in mod.get("gradients", []):
        util = grad.get("utilization_percent", "?")
        mr   = safe_float(grad.get("miss_rate"))
        rec2 = _make_rec_base(board_model, os_name, arch,
                              "realtime", "schedulability",
                              f"grad_{util}pct_mr", source, ts)
        rec2["raw_value"]    = mr
        rec2["raw_unit"]     = "ratio"
        rec2["metric_type"]  = "miss_rate"
        rec2["norm_pending"] = 0
        rec2["item_score"]   = 100.0 * (1.0 - mr) if mr is not None else None
        rec2["status"]       = "ok" if mr is not None else "null_value"
        upsert_record(conn, rec2)


def _parse_workload_module(conn, modules, board_model, os_name, arch, source, ts):
    mod = modules.get("typical-workload", {})
    if not mod:
        logger.warning("typical-workload 模块缺失")
        return

    for wl in mod.get("workloads", []):
        name = wl.get("name", "")
        cat  = WORKLOAD_CATEGORY_MAP.get(name)
        if cat is None:
            continue

        avg_ms = safe_float(wl.get("avg_time_ms"))
        rec = _make_rec_base(board_model, os_name, arch,
                             "workload", cat, name, source, ts)
        rec["raw_value"]   = avg_ms
        rec["raw_unit"]    = "ms"
        rec["metric_type"] = "latency"
        rec = _fill_R_only(rec, conn)
        upsert_record(conn, rec)


# ─────────────────────────────────────────────
# 5. Excel 解析（功耗，沿用归一化逻辑不变）
# ─────────────────────────────────────────────
def parse_power_excel(filepath: str, conn: sqlite3.Connection) -> list:
    affected = []
    try:
        xl = pd.ExcelFile(filepath, engine="openpyxl")
    except Exception as e:
        logger.error(f"打开 Excel 失败 [{filepath}]: {e}")
        return affected

    sheet_name = None
    for candidate in ["测试汇总", "Sheet1", "汇总"]:
        if candidate in xl.sheet_names:
            sheet_name = candidate
            break
    if sheet_name is None:
        logger.warning(f"找不到 '测试汇总' sheet: {filepath}")
        return affected

    try:
        df = xl.parse(sheet_name)
    except Exception as e:
        logger.error(f"解析 sheet 失败 [{filepath}]: {e}")
        return affected

    col_map = _fuzzy_col_map(df.columns, {
        "board_model": ["板卡型号", "板卡", "board"],
        "os_name":     ["操作系统", "os", "系统"],
        "task":        ["测试任务", "任务", "task"],
        "avg_power_w": ["平均功率", "平均功率(W)", "avg_power"],
        "total_j":     ["总功耗(J)", "总功耗j", "energy_j"],
        "duration_s":  ["测试总耗时", "测试总耗时(s)", "duration"],
    })

    source = os.path.basename(filepath)
    ts     = now_iso()

    for _, row in df.iterrows():
        try:
            board_model = str(row.get(col_map.get("board_model", ""), "")).strip()
            os_name     = str(row.get(col_map.get("os_name", ""), "")).strip()
            task        = str(row.get(col_map.get("task", ""), "")).strip().lower()

            if not board_model or not os_name or not task:
                continue

            arch         = "unknown"
            power_field  = POWER_TASK_MAP.get(task)
            if power_field is None:
                continue

            avg_power = safe_float(row.get(col_map.get("avg_power_w", ""), None))
            if avg_power is None:
                total_j  = safe_float(row.get(col_map.get("total_j", ""), None))
                duration = safe_float(row.get(col_map.get("duration_s", ""), None))
                if total_j and duration and duration > 0:
                    avg_power = total_j / duration

            rec = _make_rec_base(board_model, os_name, arch,
                                 "power", power_field,
                                 "avg_power_w", source, ts)
            rec["raw_value"]   = avg_power
            rec["raw_unit"]    = "W"
            rec["metric_type"] = "power"

            if rec["raw_value"] is None:
                rec["status"] = "null_value"
            else:
                bv = get_baseline_value(conn, board_model, arch,
                                        "power", power_field, "avg_power_w")
                if bv is not None and bv > 0 and rec["raw_value"] > 0:
                    rec["baseline_value"]  = bv
                    rec["baseline_source"] = "db"
                    rec["R_value"] = bv / rec["raw_value"]  # 越低功耗 R 越大
                    rec["status"]  = "ok"
                else:
                    rec["status"] = "baseline_missing"

            upsert_record(conn, rec)
            key = (board_model, os_name, arch)
            if key not in affected:
                affected.append(key)

        except Exception as e:
            logger.warning(f"处理行异常 [{filepath}]: {e}")
            continue

    conn.commit()
    return affected


def _fuzzy_col_map(columns, desired: dict) -> dict:
    result   = {}
    col_strs = [str(c) for c in columns]
    for key, candidates in desired.items():
        for cand in candidates:
            for actual in col_strs:
                if cand.lower() in actual.lower():
                    result[key] = actual
                    break
            if key in result:
                break
    return result


# ─────────────────────────────────────────────
# 6. 功耗 P_Score 归一化（跨系统 min-max）
# ─────────────────────────────────────────────
def normalize_power_for_board(board_model: str, arch: str,
                               conn: sqlite3.Connection):
    """
    Power_Ratio = 平均4项功耗比；
    P_Score = (Power_Ratio - Rmin) / (Rmax - Rmin) * 100
    跨同板卡所有 OS 的 Power_Ratio 归一化。
    """
    power_fields = ["static_power", "cpu_power", "memory_power", "file_power"]

    # 获取同板卡所有 OS
    all_os = conn.execute("""
    SELECT DISTINCT os_name FROM raw_records
    WHERE board_model=? AND arch=? AND category='power'
    """, (board_model, arch)).fetchall()

    ratios = {}   # os_name -> power_ratio
    for row in all_os:
        os_name = row["os_name"]
        field_ratios = []
        for pf in power_fields:
            r = conn.execute("""
            SELECT R_value FROM raw_records
            WHERE board_model=? AND os_name=? AND arch=?
              AND category='power' AND sub_category=? AND item_key='avg_power_w'
              AND R_value IS NOT NULL
            """, (board_model, os_name, arch, pf)).fetchone()
            if r:
                field_ratios.append(r["R_value"])
        if field_ratios:
            ratios[os_name] = sum(field_ratios) / len(field_ratios)

    if not ratios:
        return {}

    R_min = min(ratios.values())
    R_max = max(ratios.values())

    p_scores = {}
    for os_name, ratio in ratios.items():
        if R_max == R_min:
            p_scores[os_name] = {"score": None, "pending": True, "ratio": ratio}
        else:
            score = (ratio - R_min) / (R_max - R_min) * 100.0
            score = max(0.0, min(100.0, score))
            p_scores[os_name] = {"score": score, "pending": False, "ratio": ratio}

    return p_scores


# ─────────────────────────────────────────────
# 7. 增量得分计算
# ─────────────────────────────────────────────
def recalc_scores_for_target(board_model: str, os_name: str, arch: str,
                              conn: sqlite3.Connection,
                              power_scores: dict = None):
    """
    重算某个 (board_model, os_name, arch) 的所有层级得分。
    power_scores: {os_name: {score, pending, ratio}} 由外部传入（已跨系统归一化）
    """
    def q(category, sub_category):
        return conn.execute("""
        SELECT item_key, item_score, R_value, raw_value, metric_type, status
        FROM raw_records
        WHERE board_model=? AND os_name=? AND arch=?
          AND category=? AND sub_category=?
          AND status NOT IN ('null_value','error')
        """, (board_model, os_name, arch, category, sub_category)).fetchall()

    # ── 实时性能 ──────────────────────────────
    rt_sub = {}
    for key in BASE_RT_WEIGHTS:
        rows   = q("realtime", key)
        scores = [r["item_score"] for r in rows if r["item_score"] is not None]
        rt_sub[key] = sum(scores) / len(scores) if scores else None

    base_rt_score = _weighted_avg(rt_sub, BASE_RT_WEIGHTS)
    upsert_score(conn, board_model, os_name, arch,
                 "realtime", "base_realtime",
                 base_rt_score,
                 is_partial=any(v is None for v in rt_sub.values()),
                 extra=rt_sub)

    sched_row = conn.execute("""
    SELECT item_score, raw_value FROM raw_records
    WHERE board_model=? AND os_name=? AND arch=?
      AND category='realtime' AND sub_category='schedulability'
      AND item_key='average_mr'
    """, (board_model, os_name, arch)).fetchone()
    sched_score = sched_row["item_score"] if sched_row else None
    avg_mr_val  = sched_row["raw_value"]  if sched_row else None

    upsert_score(conn, board_model, os_name, arch,
                 "realtime", "schedulability",
                 sched_score,
                 extra={"average_mr": avg_mr_val})

    rt_score = _partial_weighted({
        "base": (base_rt_score, 0.5),
        "sched": (sched_score, 0.5),
    })
    upsert_score(conn, board_model, os_name, arch,
                 "total", "rt_score", rt_score,
                 is_partial=(base_rt_score is None or sched_score is None))

    # ── 典型负载 ──────────────────────────────
    cat_scores  = {}
    missing_cats = []
    for cat in WORKLOAD_CATEGORY_WEIGHTS:
        rows   = q("workload", cat)
        scores = [r["item_score"] for r in rows if r["item_score"] is not None]
        items  = {r["item_key"]: {"raw_ms": r["raw_value"],
                                   "R": r["R_value"],
                                   "score": r["item_score"]} for r in rows}
        cat_scores[cat] = sum(scores) / len(scores) if scores else None
        if cat_scores[cat] is None:
            missing_cats.append(cat)
        upsert_score(conn, board_model, os_name, arch,
                     "workload", cat, cat_scores[cat],
                     extra={"items": items})

    if CONFIG["workload_partial_mode"] and missing_cats:
        avail = {k: v for k, v in cat_scores.items() if v is not None}
        total_w = sum(WORKLOAD_CATEGORY_WEIGHTS[k] for k in avail)
        t_score = (
            sum(WORKLOAD_CATEGORY_WEIGHTS[k] * v / total_w
                for k, v in avail.items())
            if avail else None
        )
    else:
        t_score = _weighted_avg(cat_scores, WORKLOAD_CATEGORY_WEIGHTS)

    upsert_score(conn, board_model, os_name, arch,
                 "total", "t_score", t_score,
                 is_partial=bool(missing_cats),
                 extra={"missing_categories": missing_cats})

    # ── 功耗 ──────────────────────────────────
    p_info = (power_scores or {}).get(os_name, {})
    p_score       = p_info.get("score")
    norm_pending  = p_info.get("pending", True)
    power_ratio   = p_info.get("ratio")

    power_fields = ["static_power", "cpu_power", "memory_power", "file_power"]
    missing_power = []
    power_r_vals  = {}
    for pf in power_fields:
        row = conn.execute("""
        SELECT R_value FROM raw_records
        WHERE board_model=? AND os_name=? AND arch=?
          AND category='power' AND sub_category=? AND item_key='avg_power_w'
        """, (board_model, os_name, arch, pf)).fetchone()
        if row and row["R_value"] is not None:
            power_r_vals[pf] = row["R_value"]
        else:
            missing_power.append(pf)

    upsert_score(conn, board_model, os_name, arch,
                 "total", "p_score", p_score,
                 is_partial=bool(missing_power),
                 extra={
                     "power_ratio": power_ratio,
                     "power_r_vals": power_r_vals,
                     "normalization_pending": norm_pending,
                     "missing_tasks": missing_power,
                 })

    # ── 基础功能占位 ───────────────────────────
    f_score = CONFIG["default_function_score"]
    upsert_score(conn, board_model, os_name, arch,
                 "total", "f_score", f_score,
                 extra={"source": "placeholder"})

    # ── 总分 ──────────────────────────────────
    scores_4d = {"f": (f_score, 0.2), "rt": (rt_score, 0.3),
                 "t": (t_score, 0.4), "p": (p_score, 0.1)}
    total_score = _partial_weighted(scores_4d)
    is_partial  = any(v is None for v, _ in scores_4d.values())

    upsert_score(conn, board_model, os_name, arch,
                 "total", "total_score",
                 total_score, is_partial=is_partial)

    conn.commit()
    logger.info(
        f"重算完成 [{board_model} | {os_name} | {arch}] "
        f"total={round(total_score, 2) if total_score else None} "
        f"rt={round(rt_score, 2) if rt_score else None} "
        f"t={round(t_score, 2) if t_score else None}"
    )


def _weighted_avg(scores: dict, weights: dict) -> Optional[float]:
    avail   = {k: v for k, v in scores.items() if v is not None}
    if not avail:
        return None
    total_w = sum(weights[k] for k in avail)
    if total_w == 0:
        return None
    return sum(weights[k] * v / total_w for k, v in avail.items())


def _partial_weighted(items: dict) -> Optional[float]:
    """
    items = { key: (score_value, weight) }
    对有值的项重新归一化后加权求和。
    """
    avail = {k: (v, w) for k, (v, w) in items.items() if v is not None}
    if not avail:
        return None
    total_w = sum(w for _, w in avail.values())
    if total_w == 0:
        return None
    return sum(v * w / total_w for v, w in avail.values())


# ─────────────────────────────────────────────
# 8. 导出结果
# ─────────────────────────────────────────────
def export_results(board_model: str, os_name: str, arch: str,
                   conn: sqlite3.Connection) -> dict:

    def get_score(level, key):
        row = conn.execute("""
        SELECT score_value, is_partial, extra_json FROM score_cache
        WHERE board_model=? AND os_name=? AND arch=?
          AND score_level=? AND score_key=?
        """, (board_model, os_name, arch, level, key)).fetchone()
        if row:
            return {"score": row["score_value"],
                    "is_partial": bool(row["is_partial"]),
                    "extra": json.loads(row["extra_json"] or "{}")}
        return {"score": None, "is_partial": True, "extra": {}}

    def get_raw_items(category, sub_category):
        rows = conn.execute("""
        SELECT item_key, raw_value, raw_unit, R_value, item_score, norm_pending, status
        FROM raw_records
        WHERE board_model=? AND os_name=? AND arch=?
          AND category=? AND sub_category=?
        ORDER BY item_key
        """, (board_model, os_name, arch, category, sub_category)).fetchall()
        return {r["item_key"]: {
            "raw": r["raw_value"], "unit": r["raw_unit"],
            "R": r["R_value"],
            "score": r["item_score"],
            "norm_pending": bool(r["norm_pending"]),
            "status": r["status"],
        } for r in rows}

    f_info      = get_score("total", "f_score")
    base_rt     = get_score("realtime", "base_realtime")
    sched       = get_score("realtime", "schedulability")
    rt_total    = get_score("total", "rt_score")
    t_total     = get_score("total", "t_score")
    p_total     = get_score("total", "p_score")
    total_info  = get_score("total", "total_score")

    missing_baselines = [
        f"{r['category']}.{r['sub_category']}.{r['item_key']}"
        for r in conn.execute("""
        SELECT category, sub_category, item_key FROM raw_records
        WHERE board_model=? AND os_name=? AND arch=? AND status='baseline_missing'
        """, (board_model, os_name, arch)).fetchall()
    ]

    last_row = conn.execute("""
    SELECT MAX(last_updated) AS t FROM raw_records
    WHERE board_model=? AND os_name=? AND arch=?
    """, (board_model, os_name, arch)).fetchone()

    return {
        "board_model": board_model,
        "arch":        arch,
        "os_name":     os_name,
        "total_score": total_info["score"],
        "is_partial":  total_info["is_partial"],
        "scores": {
            "function": {
                "score":  f_info["score"],
                "source": "placeholder",
                "details": {k: None for k in [
                    "memory_bandwidth", "memory_latency", "small_file_iops",
                    "large_file_iops", "network_bandwidth", "network_latency",
                    "posix_compatibility"
                ]}
            },
            "realtime": {
                "score":    rt_total["score"],
                "incomplete": rt_total["is_partial"],
                "details": {
                    "base_realtime": {
                        "score":     base_rt["score"],
                        "sub_scores": base_rt["extra"],
                        "items": {
                            k: get_raw_items("realtime", k)
                            for k in BASE_RT_WEIGHTS
                        },
                    },
                    "schedulability": {
                        "score":      sched["score"],
                        "average_mr": sched["extra"].get("average_mr"),
                        "items":      get_raw_items("realtime", "schedulability"),
                    }
                }
            },
            "workload": {
                "score":    t_total["score"],
                "incomplete": t_total["is_partial"],
                "missing_categories": t_total["extra"].get("missing_categories", []),
                "details": {
                    cat: {
                        "score": get_score("workload", cat)["score"],
                        "items": get_raw_items("workload", cat),
                    }
                    for cat in WORKLOAD_CATEGORY_WEIGHTS
                }
            },
            "power": {
                "score":    p_total["score"],
                "normalization_pending": p_total["extra"].get("normalization_pending", True),
                "power_ratio": p_total["extra"].get("power_ratio"),
                "incomplete": p_total["is_partial"],
                "missing_tasks": p_total["extra"].get("missing_tasks", []),
                "details": {
                    pf: get_raw_items("power", pf)
                    for pf in ["static_power", "cpu_power",
                               "memory_power", "file_power"]
                }
            }
        },
        "completeness": {
            "missing_baselines": missing_baselines,
            "missing_scores": [
                name for name, info in [
                    ("realtime.base_realtime", base_rt),
                    ("realtime.schedulability", sched),
                    ("workload", t_total),
                    ("power", p_total),
                ] if info["score"] is None
            ]
        },
        "last_updated": last_row["t"] if last_row else now_iso(),
    }


# ─────────────────────────────────────────────
# 9. 批量入口（归一化顺序）
# ─────────────────────────────────────────────
def _collect_boards(conn) -> list:
    """获取数据库中所有 (board_model, arch) 组合。"""
    return conn.execute("""
    SELECT DISTINCT board_model, arch FROM raw_records
    """).fetchall()


def ingest_json_files(pattern: str, conn: sqlite3.Connection):
    """
    批量导入 JSON 文件：
    1. 解析所有 JSON → 写入 raw_records（只存 R_value）
    2. 对受影响板卡做 min-max 归一化
    3. 对受影响系统重算汇总分数
    """
    files    = glob.glob(pattern, recursive=True)
    if not files:
        logger.warning(f"没有找到 JSON 文件: {pattern}")
    affected = set()
    for fp in sorted(files):
        result = parse_json_results(fp, conn)
        if result:
            affected.add((result["board_model"],
                          result["os_name"],
                          result["arch"]))

    # 归一化受影响的板卡
    affected_boards = {(bm, arch) for bm, _, arch in affected}
    for bm, arch in affected_boards:
        newly = normalize_items_for_board(bm, arch, conn)
        affected.update(newly)

    # 功耗归一化
    for bm, arch in affected_boards:
        power_scores = normalize_power_for_board(bm, arch, conn)
        os_set = {os_ for b, os_, a in affected if b == bm and a == arch}
        for os_ in os_set:
            recalc_scores_for_target(bm, os_, arch, conn, power_scores)


def ingest_excel_files(pattern: str, conn: sqlite3.Connection):
    """批量导入功耗 Excel。"""
    files    = glob.glob(pattern, recursive=True)
    if not files:
        logger.warning(f"没有找到 Excel 文件: {pattern}")
    affected = set()
    for fp in sorted(files):
        for triple in parse_power_excel(fp, conn):
            affected.add(triple)

    affected_boards = {(bm, arch) for bm, _, arch in affected}
    for bm, arch in affected_boards:
        power_scores = normalize_power_for_board(bm, arch, conn)
        os_set = {os_ for b, os_, a in affected if b == bm and a == arch}
        for os_ in os_set:
            recalc_scores_for_target(bm, os_, arch, conn, power_scores)


# ─────────────────────────────────────────────
# 10. 便利 API（供外部 import 调用）
# ─────────────────────────────────────────────
def score_single_json(json_path: str, db_path: Optional[str] = None) -> dict:
    """
    一步完成单个 JSON 结果文件的评分流程：
    import -> normalize -> recalc -> export

    Args:
        json_path: rtbench_result.json 文件路径
        db_path: SQLite 数据库路径（可选，默认 CONFIG["storage_path"]）

    Returns:
        包含 total_score 等完整评分信息的 dict
    """
    db = db_path or CONFIG["storage_path"]
    os.makedirs(os.path.dirname(db) or ".", exist_ok=True)
    conn = init_db(db)

    try:
        info = parse_json_results(json_path, conn)
        if not info:
            return {"error": f"无法解析 JSON: {json_path}"}

        bm, os_name, arch = info["board_model"], info["os_name"], info["arch"]

        normalize_items_for_board(bm, arch, conn)
        power_scores = normalize_power_for_board(bm, arch, conn)
        recalc_scores_for_target(bm, os_name, arch, conn, power_scores)

        return export_results(bm, os_name, arch, conn)
    finally:
        conn.close()


# ─────────────────────────────────────────────
# 11. CLI
# ─────────────────────────────────────────────
def main():
    import argparse

    parser = argparse.ArgumentParser(description="工业 RTOS 评测评分工具（min-max版）")
    parser.add_argument("--db", default=None,
                        help="SQLite 数据库路径（覆盖默认路径）")
    sub    = parser.add_subparsers(dest="cmd")

    p_json = sub.add_parser("import-json")
    p_json.add_argument("pattern")

    p_excel = sub.add_parser("import-excel")
    p_excel.add_argument("pattern")

    p_exp = sub.add_parser("export")
    p_exp.add_argument("--board",  required=True)
    p_exp.add_argument("--os",     required=True)
    p_exp.add_argument("--arch",   required=True)
    p_exp.add_argument("--output", default=None)

    sub.add_parser("list")

    args = parser.parse_args()
    db_path = args.db or CONFIG["storage_path"]
    os.makedirs(os.path.dirname(db_path) or ".", exist_ok=True)
    conn = init_db(db_path)
    logger.info(f"数据库: {db_path}")

    if args.cmd == "import-json":
        ingest_json_files(args.pattern, conn)

    elif args.cmd == "import-excel":
        ingest_excel_files(args.pattern, conn)

    elif args.cmd == "export":
        result = export_results(args.board, args.os, args.arch, conn)
        out    = json.dumps(result, ensure_ascii=False, indent=2)
        if args.output:
            with open(args.output, "w", encoding="utf-8") as f:
                f.write(out)
            logger.info(f"结果写入: {args.output}")
        else:
            print(out)

    elif args.cmd == "list":
        rows = conn.execute("""
        SELECT DISTINCT board_model, os_name, arch FROM raw_records
        ORDER BY board_model, os_name
        """).fetchall()
        for r in rows:
            print(f"  {r['board_model']} | {r['os_name']} | {r['arch']}")
    else:
        parser.print_help()

    conn.close()


if __name__ == "__main__":
    main()
