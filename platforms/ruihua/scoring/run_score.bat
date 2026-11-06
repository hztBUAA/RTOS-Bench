@echo off
setlocal
cd /d "%~dp0"

if "%~1"=="" (
    echo Usage: drag result.json onto run_score.bat
    echo Or run: run_score.bat result.json
    pause
    exit /b 1
)

python score_rtbench.py %*
set RC=%ERRORLEVEL%
if "%RC%"=="9009" (
    py -3 score_rtbench.py %*
    set RC=%ERRORLEVEL%
)
if not "%RC%"=="0" (
    echo.
    echo Exit code: %RC%
    echo If 2, some data is missing. Check the notes above.
)
pause
exit /b %RC%
