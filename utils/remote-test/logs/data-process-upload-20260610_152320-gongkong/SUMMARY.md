# Data Process Upload Summary

## Result

- Upload status: HTTP 200
- Read-back search status: HTTP 200
- Search hit count: 1
- Resolved index: `rtos_perf_v1`
- Document `_id` / `biz_id`: `rtbench-joint-debug-20260610_152320-gongkong-testall`

## Request

- Method: `POST`
- Endpoint: `http://101.37.88.128:30000/data/api/v1/performance_safety/index`
- Dataset key: `performance_safety`
- API key: `VEsx9q...yt2L`
- Signature method: HMAC-SHA256, matching `Data Process OpenAPI` document
- Body size: 17427 bytes
- Body SHA-256: `d5858fea377f759ea02c54f6de9daea09ec945b7bc6b8a906841d8803ddca0d3`

## Sent Data

- Source result file: `utils/remote-test/logs/sylixos-entry-validation-20260427-final/gongkong_testall_20260427_233446.json`
- Saved request body: `utils/remote-test/logs/data-process-upload-20260610_152320-gongkong/sent_document.json`
- `name`: `RTOS-Bench gongkong test-all`
- `run_label`: `gongkong_testall_20260427_233446`
- `board_label`: `gongkong`
- `comparison_group`: `rtbench-sylixos-entry-validation-20260427-final`
- `podName`: `cursor-local-joint-debug-20260610_152320-gongkong`
- `upload_path`: `rtbench-results`
- `result_format`: `json`
- Included modules: `test-realtime`, `test-schedule`, `test-stress`, `test-cmd`, `typical-workload`

## Logs

- Request metadata: `utils/remote-test/logs/data-process-upload-20260610_152320-gongkong/index_request_meta.json`
- Upload response: `utils/remote-test/logs/data-process-upload-20260610_152320-gongkong/index_response.json`
- Search metadata: `utils/remote-test/logs/data-process-upload-20260610_152320-gongkong/search_request_meta.json`
- Search response: `utils/remote-test/logs/data-process-upload-20260610_152320-gongkong/search_response.json`
