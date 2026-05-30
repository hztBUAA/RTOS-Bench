# Four-Vendor Build Log Records

Date: 2026-05-29

This directory stores one reviewable build log record for each RTOS vendor:
翼辉/SylixOS, 锐华/Ruihua ReWorks, OneOS, and 东土/Dongtu Intewell. The records are
compact excerpts intended for build/execution documentation screenshots and
future traceability.

## Files

- `sylixos-feiteng-build.log`: SylixOS Feiteng compiler, linker, strip, and
  create-success excerpt.
- `ruihua-feiteng-build.log`: Ruihua ReWorks build integration and verified
  image summary. No raw compiler transcript was found during this archive pass.
- `ruihua-feiteng-build-20260530.log`: raw Ruihua/ReWorks `gnu_make clean all`
  transcript captured from the current `feiteng4rtos` sample project, including
  final `reworks.elf` size and SHA256.
- `oneos-phytium-build.log`: OneOS CMake configure context and final target
  success excerpt.
- `dongtu-orangepi-build.log`: Dongtu Intewell make/IDE build excerpt plus
  metadata with `make_exit: 0`, binary path, SHA256, and size.

## Current Evidence Level

- Raw compiler evidence is available in the archived source logs for SylixOS and
  Dongtu, then copied here as concise excerpts.
- OneOS evidence combines the configure log and the successful retry build log.
- Ruihua now has both the earlier validation summary and a raw command-line
  `gnu_make clean all` transcript from the current sample project.

## Future Capture Checklist

- Store raw compiler output under `utils/remote-test/logs/<vendor>-<board>-<date>/`.
- Keep one concise `SUMMARY.md` beside raw logs with board, RTOS, project path,
  binary path, SHA256, size, and validation command.
- Prefer filenames beginning with `build_` for compiler transcripts and
  `deploy_` or `terminal` for board execution logs.
- If a build completes after a retry, keep both the failed/partial log and the
  final success log, then point documentation at the final success record.
