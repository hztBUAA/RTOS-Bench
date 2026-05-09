import fs from "node:fs/promises";
import path from "node:path";
import { SpreadsheetFile, Workbook } from "@oai/artifact-tool";

const repoRoot = "C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench";
const logDir = `${repoRoot}/utils/remote-test/logs/sylixos-test-schedule-20260427-final`;
const outputDir = `${logDir}/outputs/sylixos_workload_timing`;
const outputXlsx = `${outputDir}/sylixos_workload_timing_quick_comparison.xlsx`;

const scheduleLogs = [
  "test_latest_cycles1_orangepi_20260427_172334.log",
  "test_latest_cycles1_gongkong_20260427_172334.log",
  "test_latest_cycles1_loongson_20260427_172334.log",
  "test_latest_cycles2_orangepi_20260427_191911.log",
  "test_latest_cycles2_gongkong_20260427_191911.log",
  "test_latest_cycles2_loongson_20260427_191911.log",
];

const quickWrapped = new Set(["mqtt", "modbus"]);
const workloadLabels = new Map([
  ["CUSUM", "cusum"],
  ["FAST", "fast"],
  ["EPNP", "epnp"],
  ["EKF", "ekf"],
  ["ICP", "icp"],
  ["PID", "pid"],
  ["EWMA", "ewma"],
  ["SCHED-MODBUS", "modbus"],
  ["SCHED-MQTT", "mqtt"],
]);

function parseBoardAndCycles(file) {
  const match = file.match(/test_latest_cycles(\d+)_([a-z0-9]+)_/i);
  if (!match) throw new Error(`Cannot parse log name: ${file}`);
  return { cycles: Number(match[1]), board: match[2] };
}

function pushSample(map, workload, value) {
  if (!map.has(workload)) map.set(workload, []);
  map.get(workload).push(value);
}

function avg(values) {
  return values.length ? values.reduce((a, b) => a + b, 0) / values.length : null;
}

function max(values) {
  return values.length ? Math.max(...values) : null;
}

function round(value, digits = 3) {
  if (value === null || value === undefined || Number.isNaN(value)) return null;
  const scale = 10 ** digits;
  return Math.round(value * scale) / scale;
}

async function parseScheduleLog(file) {
  const { board, cycles } = parseBoardAndCycles(file);
  const text = await fs.readFile(path.join(logDir, file), "utf8");
  const lines = text.split(/\r?\n/);
  const wcet = new Map();
  const periods = [];
  const phase1Samples = new Map();
  const runtimeSamples = new Map();
  let phase = "pre";
  let gradient = null;
  let avgMissRate = null;
  let finalScore = null;
  let pass = false;

  for (const line of lines) {
    if (line.includes("[Phase 1]")) phase = "wcet";
    if (line.includes("[Phase 2]")) phase = "runtime";

    let m = line.match(/\s+\[([^\]]+)\]: WCET = ([0-9.]+) ms \((\d+) iters\)/);
    if (m) {
      wcet.set(m[1].toLowerCase(), {
        workload: m[1].toLowerCase(),
        wcetMs: Number(m[2]),
        iters: Number(m[3]),
      });
      continue;
    }

    m = line.match(/\[(CUSUM|FAST|EPNP|EKF|ICP|PID|EWMA|SCHED-MODBUS|SCHED-MQTT)\]\s+samples=\d+\s+total_time=([0-9.]+)\s+ms/i);
    if (m) {
      const workload = workloadLabels.get(m[1].toUpperCase());
      if (workload) {
        pushSample(phase === "runtime" ? runtimeSamples : phase1Samples, workload, Number(m[2]));
      }
      continue;
    }

    m = line.match(/>>> Utilization Gradient:\s+(\d+)%/);
    if (m) {
      gradient = Number(m[1]);
      continue;
    }

    m = line.trim().match(/^([a-z0-9_]+)\s+\|\s+([0-9.]+)\s+\|\s+([0-9.]+)\s+\|\s+([0-9.]+)/i);
    if (m && gradient !== null) {
      periods.push({
        board,
        cycles,
        gradient,
        workload: m[1].toLowerCase(),
        wcetMs: Number(m[2]),
        utilPct: Number(m[3]),
        periodMs: Number(m[4]),
        sourceLog: file,
      });
      continue;
    }

    m = line.match(/Average Miss Rate:\s+([0-9.]+)/);
    if (m) avgMissRate = Number(m[1]);
    m = line.match(/Final Score:\s+([0-9.]+)/);
    if (m) finalScore = Number(m[1]);
    if (line.includes("[PASS]")) pass = true;
  }

  return {
    file,
    board,
    cycles,
    wcet,
    periods,
    phase1Samples,
    runtimeSamples,
    avgMissRate,
    finalScore,
    pass,
  };
}

function buildRows(parsedLogs) {
  const timingRows = [];
  const periodRows = [];
  const summaryRows = [];

  for (const parsed of parsedLogs) {
    const workloads = Array.from(parsed.wcet.keys()).sort();
    for (const p of parsed.periods) periodRows.push(p);

    const maxByGradient = new Map();
    for (const p of parsed.periods) {
      const cur = maxByGradient.get(p.gradient);
      if (!cur || p.periodMs > cur.periodMs) maxByGradient.set(p.gradient, p);
    }
    const maxPeriod = parsed.periods.reduce((best, p) => !best || p.periodMs > best.periodMs ? p : best, null);
    const estimatedWaitMin = Array.from(maxByGradient.values())
      .reduce((sum, p) => sum + p.periodMs * parsed.cycles / 1000 / 60, 0);

    summaryRows.push({
      board: parsed.board,
      cycles: parsed.cycles,
      gradients: maxByGradient.size,
      workloads: workloads.length,
      finalScore: parsed.finalScore,
      avgMissRate: parsed.avgMissRate,
      maxPeriodMs: maxPeriod?.periodMs ?? null,
      maxPeriodWorkload: maxPeriod?.workload ?? "",
      maxPeriodGradient: maxPeriod?.gradient ?? null,
      estimatedWaitMin,
      maxGradientWaitSec: maxPeriod ? maxPeriod.periodMs * parsed.cycles / 1000 : null,
      sourceLog: parsed.file,
    });

    for (const workload of workloads) {
      const wc = parsed.wcet.get(workload);
      const p30 = parsed.periods.find(p => p.workload === workload && p.gradient === 30);
      const p100 = parsed.periods.find(p => p.workload === workload && p.gradient === 100);
      const workloadPeriods = parsed.periods.filter(p => p.workload === workload);
      const maxP = workloadPeriods.reduce((best, p) => !best || p.periodMs > best.periodMs ? p : best, null);
      const phase1 = parsed.phase1Samples.get(workload) ?? [];
      const runtime = parsed.runtimeSamples.get(workload) ?? [];
      timingRows.push({
        board: parsed.board,
        cycles: parsed.cycles,
        workload,
        quickWrapper: quickWrapped.has(workload) ? "yes" : "no",
        phase1SampleAvgMs: round(avg(phase1)),
        phase1SampleMaxMs: round(max(phase1)),
        wcetMs: wc?.wcetMs ?? null,
        wcetIters: wc?.iters ?? null,
        runtimeSamples: runtime.length,
        runtimeAvgTotalMs: round(avg(runtime)),
        runtimeMaxTotalMs: round(max(runtime)),
        period30Ms: p30?.periodMs ?? null,
        period100Ms: p100?.periodMs ?? null,
        maxPeriodMs: maxP?.periodMs ?? null,
        maxPeriodGradient: maxP?.gradient ?? null,
        sourceLog: parsed.file,
      });
    }
  }

  return { timingRows, periodRows, summaryRows };
}

async function lineOf(relativePath, needle, occurrence = 1) {
  const full = path.join(repoRoot, relativePath);
  const text = await fs.readFile(full, "utf8");
  const lines = text.split(/\r?\n/);
  let seen = 0;
  for (let i = 0; i < lines.length; i++) {
    if (lines[i].includes(needle)) {
      seen++;
      if (seen === occurrence) return i + 1;
    }
  }
  return "";
}

async function codeRefs() {
  const refs = [
    ["Schedule wrapper include", "generator/test_schedule.c", "test_schedule/sched_workloads.h", "test-schedule is wired to the wrapper registry."],
    ["Wrapper lookup", "generator/test_schedule.c", "static const struct sched_workload_wrapper *get_sched_wrapper_for_workload", "Looks up workload-specific bounded execution hooks."],
    ["Wrapper dispatch", "generator/test_schedule.c", "wrapper->quick_exec();", "Uses quick_exec when a wrapper exists; otherwise falls back to original workload exec."],
    ["WCET measurement", "generator/test_schedule.c", "static uint64_t measure_wcet_ns", "WCET is measured through the same wrapper/original dispatch used at runtime."],
    ["Runtime task dispatch", "generator/test_schedule.c", "static void task_thread_entry", "Periodic task threads call the same schedule workload dispatch path."],
    ["Default cycles", "generator/test_schedule.h", "#define TEST_SCHEDULE_CYCLES", "Formal schedule default is 10000 cycles."],
    ["Quick cycles", "generator/test_schedule.h", "#define TEST_SCHEDULE_QUICK_CYCLES", "Smoke profile default is 10 cycles."],
    ["MQTT wrapper registration", "generator/test_schedule/sched_workloads.c", '.name = "mqtt"', "Registers MQTT quick wrapper and 2000 ms expected cap."],
    ["MODBUS wrapper registration", "generator/test_schedule/sched_workloads.c", '.name = "modbus"', "Registers MODBUS quick wrapper and 2000 ms expected cap."],
    ["MQTT quick message limit", "generator/test_schedule/sched_mqtt_wrapper.c", "#define SCHED_MQTT_MAX_MESSAGES", "Bounds MQTT to 50 messages instead of the full GeoLife track."],
    ["MQTT quick timeout", "generator/test_schedule/sched_mqtt_wrapper.c", "#define SCHED_MQTT_MAX_RUNTIME_US", "Bounds MQTT execution to 2 seconds."],
    ["MODBUS quick timeouts", "generator/test_schedule/sched_modbus_wrapper.c", "#define SCHED_MDB_CLIENT_WAIT_US", "Reduces client wait/retry/accept delays for schedule usage."],
    ["Original MQTT full path", "workloads/MQTT/mqtt_bench.c", "if (g_cursor >= GEOLIFE_COUNT)", "Full MQTT sends the whole GeoLife track and depends on network progress."],
    ["Original MODBUS server wait", "workloads/MODBUS/modbus_bench.c", "struct timeval server_tv = {3, 0}", "Full MODBUS has longer accept/retry timing than schedule wrapper."],
    ["SylixOS test-all parser", "generator/sylixos_entry.c", "static int parse_test_all_args", "Adds normalized test-all parser with validation and bad-option fallback."],
    ["SylixOS workload quick in test-all", "generator/sylixos_entry.c", "static void collect_workload_results", "test-all quick also uses wrappers for MQTT/MODBUS workload collection."],
    ["SylixOS test-all worker", "generator/sylixos_entry.c", "static void *test_all_thread_entry", "Runs all modules and exports JSON from a larger worker stack."],
    ["SylixOS export-result", "generator/sylixos_entry.c", 'strcmp(argv[1], "export-result")', "Adds explicit JSON export subcommand."],
    ["Version override", "generator/platform/sylixos/version_override.c", "__sylixos_version", "Overrides linked SylixOS version string for loader compatibility."],
    ["SylixOS mk sources", "platforms/sylixos/rtos-bench.mk", "sched_mqtt_wrapper.c", "Includes schedule wrappers in SylixOS builds."],
  ];

  const rows = [];
  for (const [component, file, needle, note] of refs) {
    rows.push([component, file, await lineOf(file, needle), needle, note]);
  }
  return rows;
}

function writeSheet(sheet, title, headers, rows) {
  sheet.showGridLines = false;
  sheet.getRange("A1").values = [[title]];
  sheet.getRange("A1").format.font = { bold: true, size: 16, color: "#17365D" };
  sheet.getRangeByIndexes(2, 0, 1, headers.length).values = [headers];
  if (rows.length) {
    sheet.getRangeByIndexes(3, 0, rows.length, headers.length).values = rows;
  }
  const used = sheet.getUsedRange();
  used.format.autofitColumns();
  used.format.autofitRows();
  const header = sheet.getRangeByIndexes(2, 0, 1, headers.length);
  header.format.fill = { color: "#1F4E78" };
  header.format.font = { bold: true, color: "#FFFFFF" };
  header.format.wrapText = true;
  sheet.freezePanes.freezeRows(3);
  if (rows.length) {
    const endCol = String.fromCharCode("A".charCodeAt(0) + headers.length - 1);
    sheet.tables.add(`A3:${endCol}${rows.length + 3}`, true, title.replace(/[^A-Za-z0-9]/g, "").slice(0, 24));
  }
}

function setFormulas(sheet, startRow, rowCount, colLetter, formulaFactory) {
  if (!rowCount) return;
  const formulas = [];
  for (let r = 0; r < rowCount; r++) {
    const excelRow = startRow + r;
    formulas.push([formulaFactory(excelRow)]);
  }
  sheet.getRange(`${colLetter}${startRow}:${colLetter}${startRow + rowCount - 1}`).formulas = formulas;
}

async function main() {
  const parsedLogs = [];
  for (const log of scheduleLogs) parsedLogs.push(await parseScheduleLog(log));
  const { timingRows, periodRows, summaryRows } = buildRows(parsedLogs);
  const refs = await codeRefs();

  const workbook = Workbook.create();
  const summary = workbook.worksheets.add("Summary");
  const timing = workbook.worksheets.add("Workload Timing");
  const periods = workbook.worksheets.add("Gradient Periods");
  const quick = workbook.worksheets.add("Quick Path");
  const code = workbook.worksheets.add("Code Refs");
  const logs = workbook.worksheets.add("Logs");

  const summaryHeaders = ["Board", "Cycles", "Gradients", "Workloads", "FinalScore", "AvgMissRate", "MaxPeriodMs", "MaxPeriodWorkload", "MaxPeriodGradient", "EstimatedWaitMin", "MaxGradientWaitSec", "SourceLog"];
  const summaryValues = summaryRows
    .sort((a, b) => a.cycles - b.cycles || a.board.localeCompare(b.board))
    .map(r => [r.board, r.cycles, r.gradients, r.workloads, r.finalScore, r.avgMissRate, round(r.maxPeriodMs), r.maxPeriodWorkload, r.maxPeriodGradient, round(r.estimatedWaitMin, 2), round(r.maxGradientWaitSec, 2), r.sourceLog]);
  writeSheet(summary, "Schedule Run Summary", summaryHeaders, summaryValues);
  summary.getRange("A1:L1").merge();
  summary.getRange("E4:K20").format.numberFormat = "0.00";
  const chartDataStart = 3 + summaryValues.findIndex(r => r[1] === 2) + 1;
  if (chartDataStart > 3) {
    const chart = summary.charts.add("bar", summary.getRange(`A${chartDataStart}:J${summaryValues.length + 3}`));
    chart.setPosition("N3", "V20");
    chart.title = "Estimated schedule wait by board";
    chart.hasLegend = false;
  }

  const timingHeaders = ["Board", "Cycles", "Workload", "QuickWrapper", "Phase1SampleAvgMs", "Phase1SampleMaxMs", "WCETMs", "WCETIters", "RuntimeSamples", "RuntimeAvgTotalMs", "RuntimeMaxTotalMs", "Period30Ms", "Period100Ms", "MaxPeriodMs", "MaxPeriodGradient", "OneCycleSec", "NcyclesWaitSec", "SourceLog"];
  const timingValues = timingRows
    .sort((a, b) => a.cycles - b.cycles || a.board.localeCompare(b.board) || a.workload.localeCompare(b.workload))
    .map(r => [r.board, r.cycles, r.workload, r.quickWrapper, r.phase1SampleAvgMs, r.phase1SampleMaxMs, r.wcetMs, r.wcetIters, r.runtimeSamples, r.runtimeAvgTotalMs, r.runtimeMaxTotalMs, r.period30Ms, r.period100Ms, r.maxPeriodMs, r.maxPeriodGradient, null, null, r.sourceLog]);
  writeSheet(timing, "Workload Timing", timingHeaders, timingValues);
  setFormulas(timing, 4, timingValues.length, "P", row => `=N${row}/1000`);
  setFormulas(timing, 4, timingValues.length, "Q", row => `=N${row}*B${row}/1000`);
  timing.getRange("E4:Q200").format.numberFormat = "0.000";

  const periodHeaders = ["Board", "Cycles", "GradientPct", "Workload", "WCETMs", "UtilPct", "PeriodMs", "OneCycleSec", "NcyclesSec", "SourceLog"];
  const periodValues = periodRows
    .sort((a, b) => a.cycles - b.cycles || a.board.localeCompare(b.board) || a.gradient - b.gradient || a.workload.localeCompare(b.workload))
    .map(r => [r.board, r.cycles, r.gradient, r.workload, r.wcetMs, r.utilPct, r.periodMs, null, null, r.sourceLog]);
  writeSheet(periods, "Gradient Periods", periodHeaders, periodValues);
  setFormulas(periods, 4, periodValues.length, "H", row => `=G${row}/1000`);
  setFormulas(periods, 4, periodValues.length, "I", row => `=G${row}*B${row}/1000`);
  periods.getRange("E4:I500").format.numberFormat = "0.000";

  const quickRows = [
    ["mqtt", "yes", "sched_mqtt_quick_exec", "50 messages plus 2 second cap", "Full MQTT sends the full GeoLife track through the remote broker; if broker/network stalls, schedule timing can be dominated by transport behavior.", "Preserves scheduler discrimination for a bounded MQTT job because WCET and runtime use the same path; reduces discrimination for full network throughput."],
    ["modbus", "yes", "sched_modbus_quick_exec", "same request count, shorter waits/retries, isolated port", "Full MODBUS uses longer server/client waits and port 5020; failures can add seconds or collide with prior state.", "Preserves bounded Modbus request timing; less representative of long timeout recovery behavior."],
    ["cusum/fast/epnp/ekf/icp/pid/ewma", "no", "original workload exec", "no schedule wrapper", "Same execution path with or without quick wrapper.", "Discrimination is unchanged for these workloads."],
  ];
  writeSheet(quick, "Quick Path", ["Workload", "Wrapper", "Entry", "Bounded behavior", "Without quick", "Effect on intent"], quickRows);

  writeSheet(code, "Code Refs", ["Component", "File", "Line", "Needle", "Why it matters"], refs);
  const logRows = [
    ["cycles=1 OrangePi", "PASS", "test_latest_cycles1_orangepi_20260427_172334.log"],
    ["cycles=1 Gongkong", "PASS", "test_latest_cycles1_gongkong_20260427_172334.log"],
    ["cycles=1 Loongson", "PASS", "test_latest_cycles1_loongson_20260427_172334.log"],
    ["cycles=2 OrangePi", "PASS", "test_latest_cycles2_orangepi_20260427_191911.log"],
    ["cycles=2 Gongkong", "PASS", "test_latest_cycles2_gongkong_20260427_191911.log"],
    ["cycles=2 Loongson", "PASS", "test_latest_cycles2_loongson_20260427_191911.log"],
    ["Nezha latest deploy", "FTP PASS / telnet blocked", "deploy_nezha_testall_20260427_210027.log"],
    ["Nezha test-all", "BLOCKED: telnet server full", "test_nezha_testall_20260427_210107.log"],
  ];
  writeSheet(logs, "Logs", ["Item", "Status", "Log"], logRows);

  for (const ws of workbook.worksheets.items) {
    const used = ws.getUsedRange();
    used.format.wrapText = true;
  }

  await fs.mkdir(outputDir, { recursive: true });

  const check = await workbook.inspect({
    kind: "table",
    range: "Summary!A1:L12",
    include: "values,formulas",
    tableMaxRows: 12,
    tableMaxCols: 12,
  });
  console.log(check.ndjson);

  const errors = await workbook.inspect({
    kind: "match",
    searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
    options: { useRegex: true, maxResults: 200 },
    summary: "formula error scan",
  });
  console.log(errors.ndjson);

  for (const name of ["Summary", "Workload Timing", "Gradient Periods", "Quick Path", "Code Refs", "Logs"]) {
    const preview = await workbook.render({ sheetName: name, autoCrop: "all", scale: 1, format: "png" });
    const bytes = new Uint8Array(await preview.arrayBuffer());
    await fs.writeFile(`${outputDir}/${name.replace(/[^A-Za-z0-9]/g, "_")}.png`, bytes);
  }

  const xlsx = await SpreadsheetFile.exportXlsx(workbook);
  await xlsx.save(outputXlsx);
  console.log(`OUTPUT_XLSX=${outputXlsx}`);
  process.exit(0);
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
