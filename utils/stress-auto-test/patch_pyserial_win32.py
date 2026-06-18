import os
import shutil
import sys


def patch_pyserial_win32():
    if sys.platform != "win32":
        return False, "Not Windows. Patch skipped."

    try:
        import serial.serialwin32 as serialwin32
    except Exception as exc:
        return False, f"Failed to import serial.serialwin32: {exc}"

    file_path = getattr(serialwin32, "__file__", None)
    if not file_path or not os.path.exists(file_path):
        return False, "serialwin32.py was not found."

    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    if "RTOS_BENCH_PATCH_IGNORE_SETCOMMSTATE_FAILURE" in content:
        return True, f"Patch already applied: {file_path}"

    old_block = """        if not win32.SetCommState(self._port_handle, ctypes.byref(comDCB)):
            raise SerialException(
                'Cannot configure port, something went wrong. '
                'Original message: {!r}'.format(ctypes.WinError()))"""

    new_block = """        if not win32.SetCommState(self._port_handle, ctypes.byref(comDCB)):
            # RTOS_BENCH_PATCH_IGNORE_SETCOMMSTATE_FAILURE
            # Some Windows USB virtual COM drivers return WinError 31 here after
            # the first open/close cycle, although the port is still usable.
            # This local venv patch follows the common workaround for:
            #   Cannot configure port, PermissionError(13, ..., None, 31)
            pass"""

    if old_block not in content:
        return False, (
            "Target SetCommState block was not found. "
            "pySerial version may be different. Manual check required: "
            f"{file_path}"
        )

    backup_path = file_path + ".rtos_bench_backup"

    if not os.path.exists(backup_path):
        shutil.copy2(file_path, backup_path)

    content = content.replace(old_block, new_block)

    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

    return True, f"Patch applied: {file_path}; backup: {backup_path}"


if __name__ == "__main__":
    ok, message = patch_pyserial_win32()
    print(message)
    sys.exit(0 if ok else 1)
