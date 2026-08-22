[CmdletBinding()]
param(
    [string]$EngineRoot,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'Hellwake.uproject'
$BuildScript = Join-Path $PSScriptRoot 'BuildHellwake.ps1'
$ReportDir = Join-Path $ProjectRoot 'Saved\AutomationReports'
$TestLog = Join-Path $ProjectRoot 'Saved\BuildLogs\HellwakeSmokeTests.log'

if (-not $SkipBuild) {
    & $BuildScript -EngineRoot $EngineRoot
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

function Test-UnrealRoot([string]$Root) {
    if ([string]::IsNullOrWhiteSpace($Root)) { return $false }
    return Test-Path (Join-Path $Root 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe')
}

if (-not (Test-UnrealRoot $EngineRoot)) {
    foreach ($Candidate in @(
        'C:\Program Files\Epic Games\UE_5.4',
        'C:\Epic Games\UE_5.4',
        'D:\Epic Games\UE_5.4',
        'D:\UE_5.4'
    )) {
        if (Test-UnrealRoot $Candidate) {
            $EngineRoot = $Candidate
            break
        }
    }
}

if (-not (Test-UnrealRoot $EngineRoot)) {
    throw "UnrealEditor-Cmd.exe not found. Pass -EngineRoot '<path to UE_5.4>'."
}

$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $TestLog) | Out-Null

Write-Host "=== Running Hellwake smoke tests ==="
& $EditorCmd $ProjectFile `
    -unattended `
    -nop4 `
    -nosplash `
    -NullRHI `
    -ReportExportPath="$ReportDir" `
    '-ExecCmds=Automation RunTests Hellwake.Smoke; Quit' `
    -log 2>&1 | Tee-Object -FilePath $TestLog
$EditorExit = $LASTEXITCODE

if ($EditorExit -ne 0) {
    Write-Error "Hellwake smoke-test process failed with exit code $EditorExit. Log: $TestLog"
    exit $EditorExit
}

$LogText = Get-Content -Raw -Path $TestLog
if ($LogText -match 'Automation Test Failed|Test Failed|Result=Failed') {
    Write-Error "One or more Hellwake smoke tests failed. Log: $TestLog"
    exit 2
}

Write-Host "HELLWAKE SMOKE TESTS COMPLETED" -ForegroundColor Green
Write-Host "Log: $TestLog"
Write-Host "Reports: $ReportDir"
exit 0
