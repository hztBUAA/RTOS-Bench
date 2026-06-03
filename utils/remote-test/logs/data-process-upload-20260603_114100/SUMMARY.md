# Data Process Upload Summary

## Result

- Upload status: HTTP 200
- Read-back search status: HTTP 200
- Search hit count: 1
- Resolved index: `rtos_perf_v1`
- Document `_id` / `biz_id`: `rtbench-joint-debug-20260603_114100-orangepi-testall`

## Request

- Method: `POST`
- Endpoint: `http://101.37.88.128:30000/data/api/v1/performance_safety/index`
- Dataset key: `performance_safety`
- API key: `VEsx9q...yt2L`
- Signature method: HMAC-SHA256, matching `Data Process OpenAPI` document
- Body size: 17296 bytes
- Body SHA-256: `c93288b26c8caeb153442b1b173c076f05e31b146b9e396f0ec7fc88331aba1e`

## Sent Data

- Source result file: `utils/remote-test/logs/sylixos-entry-validation-20260427-final/orangepi_testall_20260427_233446.json`
- Saved request body: `utils/remote-test/logs/data-process-upload-20260603_114100/sent_document.json`
- `name`: `RTOS-Bench SylixOS-Board`
- `project`: `RTOS-Bench`
- `source`: `rtbench`
- `podName`: `cursor-local-joint-debug-20260603_114100`
- `upload_path`: `rtbench-results`
- `result_format`: `json`
- Included modules: `test-realtime`, `test-schedule`, `test-stress`, `test-cmd`, `typical-workload`

## Logs

- Request metadata: `utils/remote-test/logs/data-process-upload-20260603_114100/request_meta.json`
- Upload response: `utils/remote-test/logs/data-process-upload-20260603_114100/response.json`
- Search metadata: `utils/remote-test/logs/data-process-upload-20260603_114100/search_request_meta.json`
- Search response: `utils/remote-test/logs/data-process-upload-20260603_114100/search_response.json`
