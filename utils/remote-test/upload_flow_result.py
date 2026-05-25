#!/usr/bin/env python3
"""Upload RTOS-Bench results to Flow/Kamala and Data Process APIs.

Flow/Kamala accepts XML files only. If a JSON result is provided, this tool wraps
it into a JUnit-compatible XML document. Data Process indexes the same result
content as a JSON document.
"""

from __future__ import annotations

import argparse
import hashlib
import hmac
import json
import os
import re
from pathlib import Path
import tempfile
import time
from typing import Any, Dict, List, Optional, Tuple
import urllib.error
import urllib.request
import uuid
import xml.etree.ElementTree as ET
import xml.sax.saxutils as xml_escape


FLOW_UPLOAD_PATH = "/kamala/api/v1/uploadFile/upload"
DATA_PROCESS_BASE_PATH = "/data/api/v1"
DEFAULT_BASE_URL = "http://101.37.88.128:30000"
DEFAULT_UPLOAD_PATH = "rtbench-results"
DEFAULT_DATA_PROCESS_DATASET_KEYS = ["performance_safety"]
DEFAULT_CASE_IDS = {
    "test-realtime": "rtbench.test-realtime",
    "test-schedule": "rtbench.test-schedule",
    "test-stress": "rtbench.test-stress",
    "test-cmd": "rtbench.test-cmd",
    "typical-workload": "rtbench.typical-workload",
}


def parse_dotenv_value(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in ("'", '"'):
        value = value[1:-1]
    return value


def load_dotenv_file(path: Path) -> None:
    if not path.exists():
        return

    for raw_line in path.read_text(encoding="utf-8-sig").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        if key and key not in os.environ:
            os.environ[key] = parse_dotenv_value(value)


def load_dotenv() -> None:
    repo_root = Path(__file__).resolve().parents[2]
    for dotenv_path in (repo_root / ".env", Path.cwd() / ".env"):
        load_dotenv_file(dotenv_path)


def parse_csv(value: Optional[str]) -> List[str]:
    if not value:
        return []
    return [item.strip() for item in value.split(",") if item.strip()]


def env_or_arg(value: Optional[str], env_name: str, label: str) -> str:
    resolved = value or os.environ.get(env_name)
    if not resolved:
        raise SystemExit(f"missing {label}; pass it explicitly or set {env_name}")
    return resolved


def build_string_to_sign(
    method: str,
    path: str,
    api_key: str,
    timestamp_ms: int,
    body_params: Dict[str, str],
) -> str:
    pieces = [f"apiKey={api_key}", f"timestamp={timestamp_ms}"]
    for key in sorted(body_params):
        pieces.append(f"{key}={body_params[key]}")
    return f"{method}\n{path}\n{'&'.join(pieces)}"


def generate_signature(
    method: str,
    path: str,
    body_params: Dict[str, str],
    api_key: str,
    secret_key: str,
    timestamp_ms: int,
) -> str:
    string_to_sign = build_string_to_sign(
        method, path, api_key, timestamp_ms, body_params
    )
    return hmac.new(
        secret_key.encode("utf-8"),
        string_to_sign.encode("utf-8"),
        hashlib.sha256,
    ).hexdigest()


def result_status(payload: dict, module_name: str) -> str:
    return (
        payload.get("modules", {})
        .get(module_name, {})
        .get("status", "skipped")
    )


def flatten_json(prefix: str, value: Any, out: Dict[str, str]) -> None:
    if value is None:
        return
    if isinstance(value, dict):
        for key, child in value.items():
            child_prefix = f"{prefix}.{key}" if prefix else str(key)
            flatten_json(child_prefix, child, out)
        return
    if isinstance(value, list):
        for index, child in enumerate(value):
            flatten_json(f"{prefix}[{index}]", child, out)
        return
    if isinstance(value, bool):
        out[prefix] = "true" if value else "false"
        return
    out[prefix] = str(value)


def case_id_env_name(module_name: str) -> str:
    normalized = "".join(ch if ch.isalnum() else "_" for ch in module_name)
    return "RTBENCH_FLOW_CASE_ID_" + normalized.upper()


def load_case_id_map(
    case_id_map_path: Optional[str], case_id_overrides: Optional[List[str]]
) -> Tuple[Dict[str, str], bool]:
    case_ids = dict(DEFAULT_CASE_IDS)
    has_custom_case_ids = False
    map_path = case_id_map_path or os.environ.get("RTBENCH_FLOW_CASE_ID_MAP")

    if map_path:
        raw_map = json.loads(Path(map_path).read_text(encoding="utf-8-sig"))
        if not isinstance(raw_map, dict):
            raise SystemExit(f"case id map must be a JSON object: {map_path}")
        for name, case_id in raw_map.items():
            case_ids[str(name)] = str(case_id)
        has_custom_case_ids = True

    for module_name in DEFAULT_CASE_IDS:
        env_value = os.environ.get(case_id_env_name(module_name))
        if env_value:
            case_ids[module_name] = env_value
            has_custom_case_ids = True

    for override in case_id_overrides or []:
        if "=" not in override:
            raise SystemExit(f"invalid --case-id value, expected module=id: {override}")
        module_name, case_id = override.split("=", 1)
        module_name = module_name.strip()
        case_id = case_id.strip()
        if not module_name or not case_id:
            raise SystemExit(f"invalid --case-id value, expected module=id: {override}")
        case_ids[module_name] = case_id
        has_custom_case_ids = True

    return case_ids, has_custom_case_ids


def json_to_junit_xml(
    json_path: Path, xml_path: Path, case_ids: Dict[str, str]
) -> None:
    raw_json = json_path.read_text(encoding="utf-8-sig")
    payload = json.loads(raw_json)
    meta = payload.get("meta", {})
    env = payload.get("env", {})
    modules = [
        "test-realtime",
        "test-schedule",
        "test-stress",
        "test-cmd",
        "typical-workload",
    ]
    skipped = sum(1 for name in modules if result_status(payload, name) == "skipped")
    timestamp = str(meta.get("test_timestamp", ""))
    duration = float(meta.get("total_duration_sec", 0.0) or 0.0)
    board = str(env.get("board", "rtbench"))

    def attr(value: object) -> str:
        return xml_escape.escape(str(value), {'"': "&quot;"})

    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        '<testsuites name="RTOS-Bench">',
        (
            f'  <testsuite name="rtbench" errors="0" failures="0" '
            f'skipped="{skipped}" tests="{len(modules)}" time="{duration:.3f}" '
            f'timestamp="{attr(timestamp)}" hostname="{attr(board)}">'
        ),
        "    <properties>",
    ]
    for key in [
        "framework_version",
        "os_name",
        "os_version",
        "board",
        "cpu_type",
        "cpu_freq_mhz",
        "cpu_core_num",
    ]:
        if key == "framework_version":
            value = meta.get(key, "")
        else:
            value = env.get(key, "")
        lines.append(f'      <property name="{attr(key)}" value="{attr(value)}"/>')
    lines.append("    </properties>")

    for name in modules:
        module = payload.get("modules", {}).get(name, {})
        module_duration = float(module.get("duration_sec", 0.0) or 0.0)
        lines.append(
            f'    <testcase classname="rtbench" name="{attr(name)}" '
            f'time="{module_duration:.3f}">'
        )
        lines.append("      <properties>")
        lines.append(
            f'        <property name="id" value="{attr(case_ids.get(name, name))}"/>'
        )
        lines.append("      </properties>")
        if result_status(payload, name) == "skipped":
            lines.append("      <skipped/>")
        lines.append("    </testcase>")

    lines.append(f"    <system-out>{xml_escape.escape(raw_json)}</system-out>")
    lines.append("  </testsuite>")
    lines.append("</testsuites>")
    xml_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def normalize_junit_case_ids(
    xml_path: Path,
    output_path: Path,
    case_ids: Dict[str, str],
    override_existing: bool,
) -> bool:
    tree = ET.parse(xml_path)
    root = tree.getroot()
    changed = False

    for testcase in root.iter("testcase"):
        name = testcase.attrib.get("name", "")
        target_case_id = case_ids.get(name, name)
        properties = testcase.find("properties")
        if properties is None:
            properties = ET.Element("properties")
            testcase.insert(0, properties)
            changed = True

        id_property = None
        for prop in properties.findall("property"):
            if prop.attrib.get("name") == "id":
                id_property = prop
                break

        if id_property is None:
            ET.SubElement(
                properties, "property", {"name": "id", "value": target_case_id}
            )
            changed = True
        elif override_existing and id_property.attrib.get("value") != target_case_id:
            id_property.set("value", target_case_id)
            changed = True

    if changed:
        tree.write(output_path, encoding="utf-8", xml_declaration=True)
    return changed


def normalize_base_url(base_url: str) -> str:
    if base_url.startswith(("http://", "https://")):
        return base_url.rstrip("/")
    return "http://" + base_url.rstrip("/")


def normalize_upload_dir(upload_dir: str) -> str:
    normalized = upload_dir.strip().replace("\\", "/").lstrip("/")
    if not normalized:
        raise SystemExit("upload path must be a relative directory")
    return normalized


def safe_id_piece(value: object, fallback: str) -> str:
    piece = re.sub(r"[^A-Za-z0-9_.-]+", "-", str(value or "").strip()).strip("-")
    return piece or fallback


def load_result_payload(input_path: Path) -> Tuple[Optional[dict], str, str]:
    raw_text = input_path.read_text(encoding="utf-8-sig")
    suffix = input_path.suffix.lower()
    if suffix == ".json":
        return json.loads(raw_text), raw_text, "json"
    if suffix == ".xml":
        return None, raw_text, "xml"
    raise SystemExit(f"unsupported result file type: {input_path}")


def build_default_biz_id(
    input_path: Path, payload: Optional[dict], raw_text: str
) -> str:
    digest = hashlib.sha256(raw_text.encode("utf-8")).hexdigest()[:12]
    board = "rtbench"
    timestamp = input_path.stem
    if payload:
        env = payload.get("env", {})
        meta = payload.get("meta", {})
        board = str(env.get("board") or board)
        timestamp = str(meta.get("test_timestamp") or timestamp)
    return "-".join(
        [
            "rtbench",
            safe_id_piece(board, "board"),
            safe_id_piece(timestamp, input_path.stem),
            digest,
        ]
    )


def build_data_process_document(
    input_path: Path,
    uploaded_artifact_path: Path,
    pod_name: str,
    upload_dir: str,
    biz_id: Optional[str],
) -> dict:
    payload, raw_text, result_format = load_result_payload(input_path)
    document: Dict[str, Any] = {
        "biz_id": biz_id or build_default_biz_id(input_path, payload, raw_text),
        "name": f"RTOS-Bench {input_path.stem}",
        "project": "RTOS-Bench",
        "source": "rtbench",
        "podName": pod_name,
        "upload_path": normalize_upload_dir(upload_dir),
        "source_file": input_path.name,
        "artifact_file": uploaded_artifact_path.name,
        "result_format": result_format,
        "raw_result": raw_text,
        "uploaded_at_ms": int(time.time() * 1000),
    }

    if payload:
        meta = payload.get("meta", {})
        env = payload.get("env", {})
        document["name"] = "RTOS-Bench " + str(
            env.get("board") or meta.get("test_timestamp") or input_path.stem
        )
        document["rtbench_result"] = payload

    return document


def encode_multipart(
    file_path: Path, fields: Dict[str, str], file_field: str = "file"
) -> Tuple[bytes, str]:
    boundary = f"----rtbench-{uuid.uuid4().hex}"
    chunks: List[bytes] = []

    for key, value in fields.items():
        chunks.append(f"--{boundary}\r\n".encode("utf-8"))
        chunks.append(
            f'Content-Disposition: form-data; name="{key}"\r\n\r\n'.encode("utf-8")
        )
        chunks.append(value.encode("utf-8"))
        chunks.append(b"\r\n")

    chunks.append(f"--{boundary}\r\n".encode("utf-8"))
    chunks.append(
        (
            f'Content-Disposition: form-data; name="{file_field}"; '
            f'filename="{file_path.name}"\r\n'
            "Content-Type: application/xml\r\n\r\n"
        ).encode("utf-8")
    )
    chunks.append(file_path.read_bytes())
    chunks.append(b"\r\n")
    chunks.append(f"--{boundary}--\r\n".encode("utf-8"))
    return b"".join(chunks), f"multipart/form-data; boundary={boundary}"


def upload_flow_file(
    base_url: str,
    api_key: str,
    secret_key: str,
    pod_name: str,
    upload_dir: str,
    file_path: Path,
    timeout: int,
    dry_run: bool,
) -> None:
    if file_path.suffix.lower() != ".xml":
        raise SystemExit(f"upload API accepts .xml only, got: {file_path}")

    base_url = normalize_base_url(base_url)
    upload_dir = normalize_upload_dir(upload_dir)
    timestamp_ms = int(time.time() * 1000)
    form_fields = {"path": upload_dir, "podName": pod_name}
    signature = generate_signature(
        "POST", FLOW_UPLOAD_PATH, form_fields, api_key, secret_key, timestamp_ms
    )
    url = base_url + FLOW_UPLOAD_PATH
    body, content_type = encode_multipart(file_path, form_fields)
    headers = {
        "X-API-Key": api_key,
        "X-Timestamp": str(timestamp_ms),
        "X-Signature": signature,
        "Content-Type": content_type,
    }

    if dry_run:
        print(f"DRY RUN url={url}")
        print(f"DRY RUN file={file_path}")
        print(f"DRY RUN podName={pod_name}")
        print(f"DRY RUN path={upload_dir}")
        print(f"DRY RUN X-Timestamp={timestamp_ms}")
        print(f"DRY RUN X-Signature={signature}")
        return

    request = urllib.request.Request(url, data=body, headers=headers, method="POST")
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            data = response.read().decode("utf-8", errors="replace")
            print(f"HTTP {response.status}")
            print(data)
    except urllib.error.HTTPError as exc:
        data = exc.read().decode("utf-8", errors="replace")
        raise SystemExit(f"upload failed: HTTP {exc.code}\n{data}") from exc


def upload_data_process_document(
    base_url: str,
    dataset_key: str,
    document: dict,
    api_key: str,
    secret_key: str,
    timeout: int,
    dry_run: bool,
) -> None:
    base_url = normalize_base_url(base_url)
    dataset_key = dataset_key.strip().strip("/")
    if not dataset_key:
        raise SystemExit("data process dataset key must not be empty")

    path = f"{DATA_PROCESS_BASE_PATH}/{dataset_key}/index"
    timestamp_ms = int(time.time() * 1000)
    body_params: Dict[str, str] = {}
    flatten_json("", document, body_params)
    signature = generate_signature(
        "POST", path, body_params, api_key, secret_key, timestamp_ms
    )
    url = base_url + path
    body = json.dumps(document, ensure_ascii=False, separators=(",", ":")).encode(
        "utf-8"
    )
    headers = {
        "X-API-Key": api_key,
        "X-Timestamp": str(timestamp_ms),
        "X-Signature": signature,
        "Content-Type": "application/json",
    }

    if dry_run:
        print(f"DRY RUN data-process url={url}")
        print(f"DRY RUN data-process datasetKey={dataset_key}")
        print(f"DRY RUN data-process biz_id={document.get('biz_id')}")
        print(f"DRY RUN data-process X-Timestamp={timestamp_ms}")
        print(f"DRY RUN data-process X-Signature={signature}")
        return

    request = urllib.request.Request(url, data=body, headers=headers, method="POST")
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            data = response.read().decode("utf-8", errors="replace")
            print(f"Data Process {dataset_key}: HTTP {response.status}")
            print(data)
    except urllib.error.HTTPError as exc:
        data = exc.read().decode("utf-8", errors="replace")
        raise SystemExit(
            f"data process upload failed for {dataset_key}: HTTP {exc.code}\n{data}"
        ) from exc


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Upload RTOS-Bench result to Flow/Kamala and Data Process"
    )
    parser.add_argument("--file", required=True, help="Result .xml or .json file")
    parser.add_argument("--xml-out", help="XML path when converting from JSON")
    parser.add_argument("--keep-xml", action="store_true", help="Keep temp XML")
    parser.add_argument(
        "--base-url",
        default=os.environ.get("RTBENCH_FLOW_BASE_URL", DEFAULT_BASE_URL),
        help="Flow/Kamala file upload base URL",
    )
    parser.add_argument("--api-key", default=os.environ.get("RTBENCH_FLOW_API_KEY"))
    parser.add_argument(
        "--secret-key", default=os.environ.get("RTBENCH_FLOW_SECRET_KEY")
    )
    parser.add_argument(
        "--pod-name",
        default=os.environ.get("RTBENCH_FLOW_POD_NAME")
        or os.environ.get("FLOW_POD_NAME"),
    )
    parser.add_argument(
        "--path",
        default=os.environ.get("RTBENCH_FLOW_UPLOAD_PATH", DEFAULT_UPLOAD_PATH),
        help="Relative upload directory; Flow prepends the execution environment path",
    )
    parser.add_argument(
        "--case-id-map",
        help="JSON object mapping testcase names to Flow platform testcase IDs",
    )
    parser.add_argument(
        "--case-id",
        action="append",
        help="Override one testcase ID as module=id; repeatable",
    )
    parser.add_argument(
        "--no-flow-upload",
        action="store_true",
        help="Skip Flow/Kamala XML file upload",
    )
    parser.add_argument(
        "--no-data-process-upload",
        action="store_true",
        help="Skip Data Process JSON index upload",
    )
    parser.add_argument(
        "--data-base-url",
        default=os.environ.get("RTBENCH_DATA_PROCESS_BASE_URL")
        or os.environ.get("RTBENCH_FLOW_BASE_URL")
        or DEFAULT_BASE_URL,
        help="Data Process OpenAPI base URL",
    )
    parser.add_argument(
        "--data-api-key",
        default=os.environ.get("RTBENCH_DATA_PROCESS_API_KEY")
        or os.environ.get("RTBENCH_FLOW_API_KEY"),
    )
    parser.add_argument(
        "--data-secret-key",
        default=os.environ.get("RTBENCH_DATA_PROCESS_SECRET_KEY")
        or os.environ.get("RTBENCH_FLOW_SECRET_KEY"),
    )
    parser.add_argument(
        "--data-dataset-key",
        action="append",
        help="Data Process datasetKey; repeatable. Defaults to performance_safety.",
    )
    parser.add_argument(
        "--biz-id",
        default=os.environ.get("RTBENCH_DATA_PROCESS_BIZ_ID"),
        help="Override Data Process biz_id for the uploaded result",
    )
    parser.add_argument("--timeout", type=int, default=60)
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    load_dotenv()
    args = parse_args()
    if args.no_flow_upload and args.no_data_process_upload:
        raise SystemExit("nothing to upload; both upload targets are disabled")

    flow_api_key = flow_secret_key = None
    if not args.no_flow_upload:
        flow_api_key = env_or_arg(args.api_key, "RTBENCH_FLOW_API_KEY", "Flow API key")
        flow_secret_key = env_or_arg(
            args.secret_key, "RTBENCH_FLOW_SECRET_KEY", "Flow secret key"
        )
    data_api_key = data_secret_key = None
    if not args.no_data_process_upload:
        data_api_key = env_or_arg(
            args.data_api_key or args.api_key,
            "RTBENCH_DATA_PROCESS_API_KEY",
            "Data Process API key",
        )
        data_secret_key = env_or_arg(
            args.data_secret_key or args.secret_key,
            "RTBENCH_DATA_PROCESS_SECRET_KEY",
            "Data Process secret key",
        )
    pod_name = (
        args.pod_name
        or os.environ.get("RTBENCH_FLOW_POD_NAME")
        or os.environ.get("FLOW_POD_NAME")
    )
    if not pod_name:
        raise SystemExit(
            "missing pod name; pass --pod-name or set RTBENCH_FLOW_POD_NAME/FLOW_POD_NAME"
        )
    data_dataset_keys = (
        args.data_dataset_key
        or parse_csv(os.environ.get("RTBENCH_DATA_PROCESS_DATASET_KEYS"))
        or DEFAULT_DATA_PROCESS_DATASET_KEYS
    )
    case_ids, has_custom_case_ids = load_case_id_map(args.case_id_map, args.case_id)
    input_path = Path(args.file)
    if not input_path.exists():
        raise SystemExit(f"result file not found: {input_path}")

    upload_path = input_path
    temp_dir: Optional[tempfile.TemporaryDirectory] = None
    if input_path.suffix.lower() == ".json":
        if not args.no_flow_upload or args.xml_out:
            if args.xml_out:
                upload_path = Path(args.xml_out)
            else:
                temp_dir = tempfile.TemporaryDirectory(prefix="rtbench-flow-")
                upload_path = Path(temp_dir.name) / f"{input_path.stem}.xml"
            json_to_junit_xml(input_path, upload_path, case_ids)
            print(f"generated XML: {upload_path}")
    elif input_path.suffix.lower() == ".xml":
        temp_dir = tempfile.TemporaryDirectory(prefix="rtbench-flow-")
        normalized_path = Path(temp_dir.name) / input_path.name
        if normalize_junit_case_ids(
            input_path, normalized_path, case_ids, has_custom_case_ids
        ):
            upload_path = normalized_path
            print(f"normalized XML testcase IDs: {upload_path}")

    errors: List[str] = []
    try:
        if not args.no_flow_upload:
            assert flow_api_key is not None and flow_secret_key is not None
            try:
                upload_flow_file(
                    args.base_url,
                    flow_api_key,
                    flow_secret_key,
                    pod_name,
                    args.path,
                    upload_path,
                    args.timeout,
                    args.dry_run,
                )
            except SystemExit as exc:
                errors.append(str(exc))
        if not args.no_data_process_upload:
            document = build_data_process_document(
                input_path, upload_path, pod_name, args.path, args.biz_id
            )
            for dataset_key in data_dataset_keys:
                assert data_api_key is not None and data_secret_key is not None
                try:
                    upload_data_process_document(
                        args.data_base_url,
                        dataset_key,
                        document,
                        data_api_key,
                        data_secret_key,
                        args.timeout,
                        args.dry_run,
                    )
                except SystemExit as exc:
                    errors.append(str(exc))
    finally:
        if temp_dir is not None and not args.keep_xml:
            temp_dir.cleanup()
    if errors:
        raise SystemExit("\n".join(errors))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
