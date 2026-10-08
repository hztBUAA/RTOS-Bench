[CmdletBinding()]
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $MakeArgs
)

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
& wsl.exe --cd $repoRoot bash './platforms/openeuler/build-aarch64.sh' @MakeArgs
if ($LASTEXITCODE -ne 0) {
    throw "openEuler AArch64 build failed with exit code $LASTEXITCODE"
}
