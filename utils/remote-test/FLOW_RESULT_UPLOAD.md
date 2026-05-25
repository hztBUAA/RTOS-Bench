# RTOS-Bench Result Upload

This branch adds the integration layer for uploading RTOS-Bench results to both
platform endpoints:

- Flow/Kamala file upload API from `功能执行结果文件上传接口说明.pdf`
- Data Process OpenAPI from `Data Process OpenAPI 接口文档.pdf`

Per the project discussion, RTOS-Bench does not split the output into separate
functional/performance subsets. The same complete result is uploaded to both
endpoints.

## Artifact Format

The upload API only accepts `.xml` files. RTOS-Bench still keeps JSON as the
canonical local result format, and can additionally emit a JUnit-compatible XML
wrapper containing the full JSON payload in `<system-out>`.

Example board command:

```sh
/apps/hzt/nezha-rtos-bench-r7 test-all \
  -o /apps/hzt/nezha_testall.json \
  --xml-output /apps/hzt/nezha_testall.xml
```

The same option is available for `export-result`:

```sh
rtbench export-result -o /tmp/rtbench_result.json --xml-output /tmp/rtbench_result.xml
```

Each `<testcase>` carries the Flow platform testcase mapping under
`properties/property[name=id]`:

```xml
<testcase classname="rtbench" name="test-realtime" time="0.000">
  <properties>
    <property name="id" value="rtbench.test-realtime"/>
  </properties>
</testcase>
```

The built-in default IDs are stable placeholders:

- `test-realtime` => `rtbench.test-realtime`
- `test-schedule` => `rtbench.test-schedule`
- `test-stress` => `rtbench.test-stress`
- `test-cmd` => `rtbench.test-cmd`
- `typical-workload` => `rtbench.typical-workload`

When the Flow platform creates official testcase IDs, pass them on the host side
with `--case-id-map`, `RTBENCH_FLOW_CASE_ID_MAP`, or repeatable
`--case-id module=id`. The uploader also normalizes direct `.xml` uploads by
adding missing testcase IDs before sending.

## Host/LAVA Upload

Run the uploader from the host, LAVA container, or jump-host environment after
the XML file is available locally:

Create a local `.env` in the repository root for platform credentials. The file
is ignored by git and is loaded automatically by `upload_flow_result.py`:

```sh
RTBENCH_FLOW_BASE_URL=http://101.37.88.128:30000
RTBENCH_FLOW_API_KEY=<provided-api-key>
RTBENCH_FLOW_SECRET_KEY=<provided-secret-key>
RTBENCH_FLOW_UPLOAD_PATH=rtbench-results
RTBENCH_DATA_PROCESS_BASE_URL=http://101.37.88.128:30000
RTBENCH_DATA_PROCESS_DATASET_KEYS=performance_safety
```

In Flow/LAVA execution environments, `FLOW_POD_NAME` is already provided and the
uploader uses it directly. On campus/manual machines, pass `--pod-name` or set
`RTBENCH_FLOW_POD_NAME` in `.env`. Flow validates that `podName` exists on the
platform, so local end-to-end Flow upload validation needs a real execution
environment pod name.

```sh
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.xml
```

By default the command sends:

- the XML artifact to `POST /kamala/api/v1/uploadFile/upload`
- the same RTOS-Bench result content to
  `POST /data/api/v1/performance_safety/index`

The Data Process document uses a deterministic `biz_id` derived from board,
test timestamp, and result hash. Override it with `--biz-id` if the platform
requires a specific ID.

Flow `path` is a relative business directory. Flow prepends the execution
environment path prefix on the server side, so the default `rtbench-results` is
intentionally not rooted with `/`.

If only a JSON file is available, the uploader can convert it before upload:

```sh
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.json \
  --xml-out /path/to/rtbench_result.xml
```

Example testcase ID override file:

```json
{
  "test-realtime": "FLOW_CASE_ID_REALTIME",
  "test-schedule": "FLOW_CASE_ID_SCHEDULE",
  "test-stress": "FLOW_CASE_ID_STRESS",
  "test-cmd": "FLOW_CASE_ID_CMD",
  "typical-workload": "FLOW_CASE_ID_WORKLOAD"
}
```

```sh
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.xml \
  --case-id-map /path/to/flow_case_ids.json
```

Use `--dry-run` to verify signature generation and request metadata without
sending the upload request.

Useful target controls:

```sh
# Send only the Data Process JSON index request.
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.json \
  --no-flow-upload

# Send only the Flow/Kamala XML file upload request.
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.xml \
  --no-data-process-upload

# Also index to another Data Process dataset if needed.
python utils/remote-test/upload_flow_result.py \
  --file /path/to/rtbench_result.json \
  --data-dataset-key performance_safety \
  --data-dataset-key functional_safety
```
