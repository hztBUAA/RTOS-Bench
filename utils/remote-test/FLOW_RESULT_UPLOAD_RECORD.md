# RTOS-Bench Result Upload Record

## Scope

This records the result upload integration tracked under
`feature/result-upload-flow-platform`.

RTOS-Bench keeps a single complete result artifact. Per project-owner guidance,
the upload flow does not split the result into functional, performance, or power
subsets. The same complete result is sent to both platform-side endpoints.

## Upload Targets

- Flow/Kamala file upload:
  `POST /kamala/api/v1/uploadFile/upload`
- Data Process OpenAPI index:
  `POST /data/api/v1/performance_safety/index`

The Flow/Kamala side receives the JUnit-compatible XML artifact. The Data
Process side receives the same RTOS-Bench result content as a signed JSON
document with a deterministic `biz_id`.

## Runtime Inputs

- `RTBENCH_FLOW_BASE_URL`
- `RTBENCH_FLOW_API_KEY`
- `RTBENCH_FLOW_SECRET_KEY`
- `RTBENCH_FLOW_UPLOAD_PATH`
- `RTBENCH_DATA_PROCESS_BASE_URL`
- `RTBENCH_DATA_PROCESS_DATASET_KEYS`
- `FLOW_POD_NAME` or `RTBENCH_FLOW_POD_NAME`

Local credentials live in `.env`, which is intentionally ignored by git.
Flow/LAVA environments provide `FLOW_POD_NAME`; campus/manual machines should
pass `--pod-name` or set `RTBENCH_FLOW_POD_NAME`.

## Validation

- `python -m py_compile utils/remote-test/upload_flow_result.py`
- `git diff --check`
- Dual-target `--dry-run` signature generation
- XML testcase ID generation under `testcase/properties/property[name=id]`
- Real Data Process write to `performance_safety`, returning `HTTP 200`

Flow/Kamala real upload was not completed from the local machine because the
service validates that `podName` exists in the platform execution environment.
Using the real `FLOW_POD_NAME` inside Flow/LAVA is expected to satisfy that
server-side check.
