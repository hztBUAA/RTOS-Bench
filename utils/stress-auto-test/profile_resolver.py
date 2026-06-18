from copy import deepcopy

import config


POWER_REQUIRED_FIELDS = {
    "voltage",
    "current",
    "protect_voltage",
    "protect_current"
}

SUPPORTED_DUT_CONNECTION_TYPES = {"TELNET", "COM"}


def _lookup_case_insensitive(mapping, key):
    if key in mapping:
        return key, mapping[key]

    normalized = str(key).casefold()
    for candidate, value in mapping.items():
        if str(candidate).casefold() == normalized:
            return candidate, value

    return None, None


def normalize_dut_connection_type(value):
    connection_type = str(value).strip().upper()

    if connection_type == "SERIAL":
        connection_type = "COM"

    if connection_type not in SUPPORTED_DUT_CONNECTION_TYPES:
        supported = ", ".join(sorted(SUPPORTED_DUT_CONNECTION_TYPES))
        raise ValueError(
            f"Unsupported DUT connection type '{value}'. Supported types: {supported}."
        )

    return connection_type


def get_board_profile(board_name):
    profiles = getattr(config, "PLATFORM_PROFILES", {})
    resolved_name, board_profile = _lookup_case_insensitive(profiles, board_name)

    if board_profile is None:
        available = ", ".join(sorted(profiles.keys()))
        raise KeyError(
            f"Board profile '{board_name}' was not found. Available boards: {available}."
        )

    if not isinstance(board_profile, dict):
        raise TypeError(f"Board profile '{resolved_name}' must be a dictionary.")

    return resolved_name, board_profile


def get_power_profile(board_name):
    resolved_name, board_profile = get_board_profile(board_name)
    power_profile = board_profile.get("power")

    if not isinstance(power_profile, dict):
        raise KeyError(f"Board '{resolved_name}' is missing the 'power' profile.")

    missing = sorted(POWER_REQUIRED_FIELDS - set(power_profile.keys()))
    if missing:
        raise KeyError(
            f"Board '{resolved_name}' power profile is missing fields: {missing}."
        )

    return deepcopy(power_profile)


def get_platform_profile(board_name, os_name):
    resolved_board_name, board_profile = get_board_profile(board_name)
    platforms = board_profile.get("platforms")

    if not isinstance(platforms, dict):
        raise KeyError(
            f"Board '{resolved_board_name}' is missing the 'platforms' mapping."
        )

    resolved_os_name, platform_profile = _lookup_case_insensitive(
        platforms,
        os_name
    )

    if platform_profile is None:
        available = ", ".join(sorted(platforms.keys())) or "none"
        raise KeyError(
            f"Platform profile '{os_name}' was not found for board "
            f"'{resolved_board_name}'. Available platforms: {available}."
        )

    if not isinstance(platform_profile, dict):
        raise TypeError(
            f"Platform profile '{resolved_board_name}/{resolved_os_name}' "
            "must be a dictionary."
        )

    return resolved_board_name, resolved_os_name, deepcopy(platform_profile)


def get_dut_connection_config(platform_profile, connection_type=None):
    selected_type = connection_type or platform_profile.get("dut_conn_type")
    if not selected_type:
        raise KeyError("Platform profile is missing 'dut_conn_type'.")

    selected_type = normalize_dut_connection_type(selected_type)
    all_params = platform_profile.get("dut_conn_params")

    if not isinstance(all_params, dict):
        raise KeyError("Platform profile is missing 'dut_conn_params'.")

    resolved_key, selected_params = _lookup_case_insensitive(
        all_params,
        selected_type
    )

    if selected_params is None:
        available = ", ".join(sorted(all_params.keys())) or "none"
        raise KeyError(
            f"No DUT parameters configured for '{selected_type}'. "
            f"Available parameter groups: {available}."
        )

    if not isinstance(selected_params, dict):
        raise TypeError(
            f"DUT parameter group '{resolved_key}' must be a dictionary."
        )

    if selected_type == "TELNET" and not selected_params.get("ip"):
        raise KeyError("TELNET parameters are missing 'ip'.")

    if selected_type == "COM" and not selected_params.get("port"):
        raise KeyError("COM parameters are missing 'port'.")

    return selected_type, deepcopy(selected_params)


def get_power_supply_config():
    power_supply = getattr(config, "POWER_SUPPLY_CONFIG", None)

    if not isinstance(power_supply, dict):
        raise KeyError("POWER_SUPPLY_CONFIG is missing or invalid.")

    if not power_supply.get("port"):
        raise KeyError("POWER_SUPPLY_CONFIG is missing 'port'.")

    return deepcopy(power_supply)


def resolve_task(task=None, connection_type=None):
    task = deepcopy(task if task is not None else config.CURRENT_TASK)

    board_name = task.get("board")
    os_name = task.get("os", task.get("platform"))

    if not board_name:
        raise KeyError("CURRENT_TASK is missing 'board'.")
    if not os_name:
        raise KeyError("CURRENT_TASK is missing 'os' or 'platform'.")

    resolved_board_name, resolved_os_name, platform_profile = get_platform_profile(
        board_name,
        os_name
    )

    power_profile = get_power_profile(resolved_board_name)
    dut_conn_type, dut_conn_params = get_dut_connection_config(
        platform_profile,
        connection_type=connection_type
    )

    return {
        "task": task,
        "board_name": resolved_board_name,
        "os_name": resolved_os_name,
        "power_profile": power_profile,
        "platform_profile": platform_profile,
        "dut_conn_type": dut_conn_type,
        "dut_conn_params": dut_conn_params,
        "power_supply_config": get_power_supply_config()
    }
