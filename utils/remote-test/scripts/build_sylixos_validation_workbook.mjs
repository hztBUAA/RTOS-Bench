import fs from "node:fs/promises";
import fssync from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { SpreadsheetFile, Workbook } from "@oai/artifact-tool";

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../../..");
const defaultLogDir = path.join(repoRoot, "utils/remote-test/logs/sylixos-entry-validation-20260427-final");

function argValue(name, fallback = null) {
  const index = process.argv.indexOf(name);
  if (index >= 0 && index + 1 < process.argv.length) return process.argv[index + 1];
  return fallback;
}

const logDir = path.resolve(argValue("--log-dir", defaultLogDir));
const outputDir = path.join(logDir, "outputs");
const outputXlsx = path.join(outputDir, "sylixos_schedule_validation_20260427.xlsx");
const renderPreviews = !process.argv.includes("--no-render");

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

function round(value, digits = 3) {
  if (value === null || value === undefined || Number.isNaN(value)) return null;
  const scale = 10 ** digits;
  return Math.round(value * scale) / scale;
}

function minutes(ms) {
  return round(ms / 60000, 2);
}

async function readText(file) {
  return fs.readFile(file, "utf8");
}

async function readJson(file) {
  const text = await readText(file);
  return JSON.parse(text.replace(/^\uFEFF/, ""));
}

async function latestBatch(logDirPath) {
  const batchArg = argValue("--batch");
  if (batchArg) return batchArg;
  const files = (await fs.readdir(logDirPath))
    .filter((name) => /^entry_validation_summary_\d{8}_\d{6}\.json$/.test(name))
    .map((name) => ({
      name,
      time: fssync.statSync(path.join(logDirPath, name)).mtimeMs,
      batch: name.match(/^entry_validation_summary_(\d{8}_\d{6})\.json$/)[1],
    }))
    .sort((a, b) => b.time - a.time);
  if (!files.length) throw new Error(`No entry_validation_summary_*.json in ${logDirPath}`);
  return files[0].batch;
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

async function parseScheduleLog(board, batch) {
  const logFile = path.join(logDir, `entry_validation_${board}_${batch}.log`);
  const text = await readText(logFile);
  const commandIndex = text.lastIndexOf("test-schedule --cycles");
  const scheduleText = commandIndex >= 0 ? text.slice(commandIndex) : text;
  const lines = scheduleText.split(/\r?\n/);

  const wcet = new Map();
  const periods = [];
  const phase1Samples = new Map();
  const runtimeSamples = new Map();
  let phase = "pre";
  let gradient = null;
  let avgMissRate = null;
  let finalScore = null;

  for (const line of lines) {
    if (line.includes("[Phase 1]")) phase = "wcet";
    if (line.includes("[Phase 2]")) phase = "runtime";

    let match = line.match(/\s+\[([a-z0-9_-]+)\]: WCET = ([0-9.]+) ms \((\d+) iters\)/i);
    if (match) {
      wcet.set(match[1].toLowerCase(), {
        workload: match[1].toLowerCase(),
        wcetMs: Number(match[2]),
        iters: Number(match[3]),
      });
      continue;
    }

    match = line.match(/\[(CUSUM|FAST|EPNP|EKF|ICP|PID|EWMA|SCHED-MODBUS|SCHED-MQTT)\]\s+samples=\d+\s+total_time=([0-9.]+)\s+ms/i);
    if (match) {
      const workload = workloadLabels.get(match[1].toUpperCase());
      if (workload) pushSample(phase === "runtime" ? runtimeSamples : phase1Samples, workload, Number(match[2]));
      continue;
    }

    match = line.match(/>>> Utilization Gradient:\s+(\d+)%/);
    if (match) {
      gradient = Number(match[1]);
      continue;
    }

    match = line.trim().match(/^([a-z0-9_-]+)\s+\|\s+([0-9.]+)\s+\|\s+([0-9.]+)\s+\|\s+([0-9.]+)/i);
    if (match && gradient !== null) {
      periods.push({
        board,
        gradient,
        workload: match[1].toLowerCase(),
        wcetMs: Number(match[2]),
        utilPct: Number(match[3]),
        periodMs: Number(match[4]),
        sourceLog: path.basename(logFile),
      });
      continue;
    }

    match = line.match(/Average Miss Rate:\s+([0-9.]+)/);
    if (match) avgMissRate = Number(match[1]);
    match = line.match(/Final Score:\s+([0-9.]+)/);
    if (match) finalScore = Number(match[1]);
  }

  return {
    board,
    logFile,
    wcet,
    periods,
    phase1Samples,
    runtimeSamples,
    avgMissRate,
    finalScore,
  };
}

async function lineOf(relativePath, needle, occurrence = 1) {
  const full = path.join(repoRoot, relativePath);
  const text = await readText(full);
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
    ["Formal schedule gradients", "generator/test_schedule.h", "#define TEST_SCHEDULE_UTIL_START", "Default formal schedule starts at 30%."],
    ["Formal schedule gradients", "generator/test_schedule.h", "#define TEST_SCHEDULE_UTIL_END", "Default formal schedule ends at 100%."],
    ["Formal schedule gradients", "generator/test_schedule.h", "#define TEST_SCHEDULE_UTIL_STEP", "Default formal schedule step is 10%, for 8 gradients total."],
    ["Formal default cycles", "generator/test_schedule.h", "#define TEST_SCHEDULE_CYCLES", "Naked test-schedule/test-all schedule default is 10000 cycles."],
    ["Quick schedule defaults", "generator/test_schedule.h", "#define TEST_SCHEDULE_QUICK_CYCLES", "Quick schedule defaults to 10 cycles."],
    ["Schedule stack", "generator/test_schedule.c", "#define SCHED_POSIX_STACK_SIZE", "Raises POSIX schedule task stack to 4 MB."],
    ["Schedule wrapper lookup", "generator/test_schedule.c", "return sched_get_wrapper(wl->name);", "Finds bounded wrappers for selected workloads."],
    ["Schedule wrapper dispatch", "generator/test_schedule.c", "wrapper->quick_exec();", "WCET/runtime dispatch uses wrapper when present."],
    ["Default schedule entry", "generator/test_schedule.c", "return test_schedule_run_custom(TEST_SCHEDULE_CYCLES", "Default entry forwards formal constants."],
    ["MQTT wrapper cap", "generator/test_schedule/sched_mqtt_wrapper.c", "#define SCHED_MQTT_MAX_MESSAGES", "Bounds MQTT to 50 messages."],
    ["MQTT wrapper timeout", "generator/test_schedule/sched_mqtt_wrapper.c", "#define SCHED_MQTT_MAX_RUNTIME_US", "Bounds MQTT quick runtime to 2 seconds."],
    ["Test-all parser", "generator/sylixos_entry.c", "static int parse_test_all_args", "Normalizes test-all arguments and rejects unknown options."],
    ["Test-all quick schedule", "generator/sylixos_entry.c", "params->schedule_cycles = TEST_SCHEDULE_QUICK_CYCLES", "Quick mode changes test-all schedule defaults unless explicitly overridden."],
    ["Test-all workload rounds", "generator/sylixos_entry.c", "int rounds = quick_mode ? 5 : 10;", "Quick workload collection uses 5 rounds; non-quick uses 10."],
    ["Test-all workload wrapper", "generator/sylixos_entry.c", "(quick_mode && w && w->name) ? sched_get_wrapper(w->name) : NULL", "Only test-all quick workload collection swaps to wrappers."],
    ["Export JSON fix", "generator/result_export.c", "JSON_APPEND(\"      \\\"duration_sec\\\": %.3f\", r->realtime.duration_sec);", "Avoids trailing comma when export-result is called before any module ran."],
    ["Validation JSON parse", "utils/remote-test/scripts/run_sylixos_entry_validation.py", "def validate_json_file", "Remote validation now strictly parses fetched JSON files."],
    ["SylixOS version override", "generator/platform/sylixos/version_override.c", "const char __sylixos_version[]", "Keeps SDK version symbol compatible with deployed loader."],
    ["SylixOS mk wrappers", "platforms/sylixos/rtos-bench.mk", "sched_mqtt_wrapper.c", "Includes schedule wrappers in SylixOS builds."],
    ["SylixOS mk version override", "platforms/sylixos/rtos-bench.mk", "version_override.c", "Includes SylixOS version override source."],
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
  used.format.wrapText = true;
  const header = sheet.getRangeByIndexes(2, 0, 1, headers.length);
  header.format.fill = { color: "#1F4E78" };
  header.format.font = { bold: true, color: "#FFFFFF" };
  header.format.wrapText = true;
  sheet.freezePanes.freezeRows(3);
  if (rows.length) {
    const endCol = columnLetter(headers.length);
    sheet.tables.add(`A3:${endCol}${rows.length + 3}`, true, title.replace(/[^A-Za-z0-9]/g, "").slice(0, 24));
  }
}

function columnLetter(count) {
  let n = count;
  let label = "";
  while (n > 0) {
    const rem = (n - 1) % 26;
    label = String.fromCharCode(65 + rem) + label;
    n = Math.floor((n - 1) / 26);
  }
  return label;
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

function buildRows(summary, parsedByBoard, batch) {
  const summaryRows = [];
  const timingRows = [];
  const periodRows = [];
  const cmdRows = [];

  for (const [board, result] of Object.entries(summary).sort()) {
    const parsed = parsedByBoard.get(board);
    const logStat = fssync.statSync(parsed.logFile);
    const testallFile = path.join(logDir, `${board}_testall_${batch}.json`);
    const testallStat = fssync.existsSync(testallFile) ? fssync.statSync(testallFile) : null;
    const testallJson = testallStat ? JSON.parse(fssync.readFileSync(testallFile, "utf8")) : null;
    const checks = result.checks || [];
    const maxPeriod = parsed.periods.reduce((best, p) => (!best || p.periodMs > best.periodMs ? p : best), null);
    const gradients = new Set(parsed.periods.map((p) => p.gradient));
    const fullMin = minutes(logStat.mtimeMs - logStat.birthtimeMs);
    const preScheduleMin = testallStat ? minutes(testallStat.mtimeMs - logStat.birthtimeMs) : null;
    const scheduleMin = testallStat ? minutes(logStat.mtimeMs - testallStat.mtimeMs) : null;
    const jsonChecksOk = checks.filter((c) => String(c.name || "").startsWith("json_parse:")).every((c) => c.ok);

    summaryRows.push([
      board,
      result.ok ? "PASS" : "FAIL",
      checks.filter((c) => c.ok).length,
      checks.length,
      jsonChecksOk ? "PASS" : "FAIL",
      3,
      gradients.size,
      parsed.finalScore,
      parsed.avgMissRate,
      fullMin,
      preScheduleMin,
      scheduleMin,
      maxPeriod ? round(maxPeriod.periodMs) : null,
      maxPeriod ? maxPeriod.workload : "",
      maxPeriod ? maxPeriod.gradient : null,
      testallJson?.meta?.total_duration_sec ?? null,
      path.basename(parsed.logFile),
    ]);

    const quickWorkloads = new Map();
    for (const row of testallJson?.modules?.["typical-workload"]?.workloads || []) {
      quickWorkloads.set(row.name, row);
    }
    const cmd = testallJson?.modules?.["test-cmd"];
    for (const command of cmd?.commands || []) {
      cmdRows.push([board, command.name, command.command, command.supported ? "yes" : "no"]);
    }

    for (const p of parsed.periods) {
      periodRows.push([
        p.board,
        3,
        p.gradient,
        p.workload,
        p.wcetMs,
        p.utilPct,
        p.periodMs,
        null,
        null,
        p.sourceLog,
      ]);
    }

    for (const workload of Array.from(parsed.wcet.keys()).sort()) {
      const wc = parsed.wcet.get(workload);
      const workloadPeriods = parsed.periods.filter((p) => p.workload === workload);
      const p30 = workloadPeriods.find((p) => p.gradient === 30);
      const p100 = workloadPeriods.find((p) => p.gradient === 100);
      const maxP = workloadPeriods.reduce((best, p) => (!best || p.periodMs > best.periodMs ? p : best), null);
      const q = quickWorkloads.get(workload);
      const phase1 = parsed.phase1Samples.get(workload) || [];
      const runtime = parsed.runtimeSamples.get(workload) || [];
      timingRows.push([
        board,
        workload,
        quickWrapped.has(workload) ? "yes" : "no",
        wc?.wcetMs ?? null,
        wc?.iters ?? null,
        q?.rounds ?? null,
        q?.avg_time_ms ?? null,
        q?.exec_time_ms ?? null,
        round(avg(phase1)),
        round(max(phase1)),
        runtime.length,
        round(avg(runtime)),
        round(max(runtime)),
        p30?.periodMs ?? null,
        p100?.periodMs ?? null,
        maxP?.periodMs ?? null,
        maxP?.gradient ?? null,
        null,
        null,
        path.basename(parsed.logFile),
      ]);
    }
  }

  return { summaryRows, timingRows, periodRows, cmdRows };
}

async function main() {
  const batch = await latestBatch(logDir);
  const summaryPath = path.join(logDir, `entry_validation_summary_${batch}.json`);
  const summary = await readJson(summaryPath);
  const parsedByBoard = new Map();
  for (const board of Object.keys(summary)) {
    parsedByBoard.set(board, await parseScheduleLog(board, batch));
  }

  const { summaryRows, timingRows, periodRows, cmdRows } = buildRows(summary, parsedByBoard, batch);
  const refs = await codeRefs();

  const workbook = Workbook.create();
  const summarySheet = workbook.worksheets.add("Summary");
  const timingSheet = workbook.worksheets.add("Workload Timing");
  const periodsSheet = workbook.worksheets.add("Period Matrix");
  const quickSheet = workbook.worksheets.add("Quick Path");
  const codeSheet = workbook.worksheets.add("Code Refs");
  const cmdSheet = workbook.worksheets.add("Cmd Export");
  const logsSheet = workbook.worksheets.add("Logs");

  writeSheet(summarySheet, "SylixOS Final Validation Summary", [
    "Board", "Result", "ChecksPass", "ChecksTotal", "JsonParse", "ScheduleCycles", "Gradients",
    "FinalScore", "AvgMissRate", "FullValidationMin", "BeforeScheduleMin", "ScheduleApproxMin",
    "MaxPeriodMs", "MaxPeriodWorkload", "MaxPeriodGradient", "TestAllQuickDurationSec", "ScheduleLog",
  ], summaryRows);
  summarySheet.getRange("H4:P20").format.numberFormat = "0.00";
  const chart = summarySheet.charts.add("bar", summarySheet.getRange(`A3:L${summaryRows.length + 3}`));
  chart.setPosition("S3", "AA18");
  chart.title = "Approx schedule runtime by board";
  chart.hasLegend = false;

  writeSheet(timingSheet, "Workload Timing", [
    "Board", "Workload", "ScheduleWrapper", "WCETMs", "WCETIters", "TestAllQuickRounds",
    "TestAllQuickAvgMs", "TestAllQuickTotalMs", "Phase1SampleAvgMs", "Phase1SampleMaxMs",
    "RuntimeSamples", "RuntimeAvgTotalMs", "RuntimeMaxTotalMs", "Period30Ms", "Period100Ms",
    "MaxPeriodMs", "MaxPeriodGradient", "OneCycleSec", "Cycles3Sec", "SourceLog",
  ], timingRows);
  setFormulas(timingSheet, 4, timingRows.length, "R", (row) => `=P${row}/1000`);
  setFormulas(timingSheet, 4, timingRows.length, "S", (row) => `=P${row}*3/1000`);
  timingSheet.getRange("D4:S300").format.numberFormat = "0.000";

  writeSheet(periodsSheet, "Period Matrix", [
    "Board", "Cycles", "GradientPct", "Workload", "WCETMs", "UtilPct", "PeriodMs",
    "OneCycleSec", "Cycles3Sec", "SourceLog",
  ], periodRows);
  setFormulas(periodsSheet, 4, periodRows.length, "H", (row) => `=G${row}/1000`);
  setFormulas(periodsSheet, 4, periodRows.length, "I", (row) => `=G${row}*B${row}/1000`);
  periodsSheet.getRange("E4:I500").format.numberFormat = "0.000";

  const quickRows = [
    ["Final acceptance schedule", "No --quick", "test-schedule --cycles 3", "8 gradients: 30,40,50,60,70,80,90,100", "Cycles intentionally bounded at 3 for runtime; parser/schedule path is the formal non-quick path."],
    ["Naked schedule default", "No --quick", "test-schedule", "8 gradients, 10000 cycles", "Functionally valid but impractical for this board set; wall time would scale roughly with cycles."],
    ["test-all smoke command", "--quick plus explicit overrides", "test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick", "One 30% gradient, one schedule cycle, all-quick stress, workload rounds=5", "Used only for entry/export smoke, not schedule acceptance."],
    ["test-all quick default", "--quick", "test-all --quick", "Quick constants: cycles=10, util 30-60 step30", "Still exercises all modules but uses bounded defaults."],
    ["MQTT/MODBUS schedule wrappers", "Always in test-schedule when wrapper exists", "sched_get_wrapper + quick_exec", "Network workloads are bounded for WCET/runtime", "Keeps schedule comparable and avoids transport timeout dominance; full network behavior remains outside schedule acceptance."],
    ["test-all workload collection", "Only when quick_mode", "collect_workload_results(quick_mode)", "MQTT/MODBUS use wrappers only in quick workload collection", "Non-quick test-all workload collection would run original workload exec 10 rounds."],
  ];
  writeSheet(quickSheet, "Quick Path", ["Area", "Quick Flag", "Command/Code", "What Is Reduced", "Acceptance Meaning"], quickRows);

  writeSheet(codeSheet, "Code Refs", ["Component", "File", "Line", "Needle", "Why It Matters"], refs);

  writeSheet(cmdSheet, "Cmd Export", ["Board", "Name", "Command", "Supported"], cmdRows);

  const logRows = [];
  for (const name of (await fs.readdir(logDir)).sort()) {
    const full = path.join(logDir, name);
    const stat = fssync.statSync(full);
    if (stat.isFile()) logRows.push([name, stat.size, new Date(stat.mtimeMs).toISOString(), full]);
  }
  writeSheet(logsSheet, "Logs", ["File", "Bytes", "LastWriteTime", "AbsolutePath"], logRows);

  for (const ws of workbook.worksheets.items) {
    const used = ws.getUsedRange();
    used.format.wrapText = true;
  }

  await fs.mkdir(outputDir, { recursive: true });
  const inspect = await workbook.inspect({
    kind: "table",
    range: "Summary!A1:Q10",
    include: "values,formulas",
    tableMaxRows: 10,
    tableMaxCols: 17,
  });
  console.log(inspect.ndjson);

  const errors = await workbook.inspect({
    kind: "match",
    searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
    options: { useRegex: true, maxResults: 200 },
    summary: "formula error scan",
  });
  console.log(errors.ndjson);

  if (renderPreviews) {
    for (const name of ["Summary", "Workload Timing", "Period Matrix", "Quick Path", "Code Refs", "Cmd Export", "Logs"]) {
      const preview = await workbook.render({ sheetName: name, autoCrop: "all", scale: 1, format: "png" });
      const bytes = new Uint8Array(await preview.arrayBuffer());
      await fs.writeFile(path.join(outputDir, `${name.replace(/[^A-Za-z0-9]/g, "_")}.png`), bytes);
    }
  }

  const xlsx = await SpreadsheetFile.exportXlsx(workbook);
  await xlsx.save(outputXlsx);
  console.log(`OUTPUT_XLSX=${outputXlsx}`);
}

main()
  .then(() => process.exit(0))
  .catch((err) => {
    console.error(err);
    process.exit(1);
  });
