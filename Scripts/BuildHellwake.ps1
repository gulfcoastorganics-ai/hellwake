[CmdletBinding()]
param(
    [string]$EngineRoot,
    [ValidateSet('Development','DebugGame','Shipping')]
    [string]$Configuration = 'Development',
    [switch]$LaunchEditor,
    [switch]$SkipProjectFiles
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'Hellwake.uproject'
$LogDir = Join-Path $ProjectRoot 'Saved\BuildLogs'
$BuildLog = Join-Path $LogDir ('HellwakeEditor-{0:yyyyMMdd-HHmmss}.log' -f (Get-Date))

if (-not (Test-Path $ProjectFile)) {
    throw "Hellwake.uproject not found at $ProjectFile"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function Test-UnrealRoot([string]$Root) {
    if ([string]::IsNullOrWhiteSpace($Root)) { return $false }
    return (Test-Path (Join-Path $Root 'Engine\Build\BatchFiles\Build.bat')) -and
           (Test-Path (Join-Path $Root 'Engine\Binaries\Win64\UnrealEditor.exe'))
}

function Resolve-UnrealRoot {
    param([string]$ExplicitRoot)

    if (Test-UnrealRoot $ExplicitRoot) {
        return (Resolve-Path $ExplicitRoot).Path
    }

    $Candidates = New-Object System.Collections.Generic.List[string]

    foreach ($RegistryPath in @(
        'HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.4',
        'HKLM:\SOFTWARE\WOW6432Node\EpicGames\Unreal Engine\5.4'
    )) {
        try {
            $InstallLocation = (Get-ItemProperty -Path $RegistryPath -Name InstalledDirectory -ErrorAction Stop).InstalledDirectory
            if ($InstallLocation) { $Candidates.Add($InstallLocation) }
        } catch { }
    }

    foreach ($Path in @(
        'C:\Program Files\Epic Games\UE_5.4',
        'C:\Epic Games\UE_5.4',
        'D:\Epic Games\UE_5.4',
        'D:\UE_5.4'
    )) {
        $Candidates.Add($Path)
    }

    foreach ($Candidate in $Candidates | Select-Object -Unique) {
        if (Test-UnrealRoot $Candidate) {
            return (Resolve-Path $Candidate).Path
        }
    }

    throw @"
Unreal Engine 5.4 was not found automatically.
Run this script again with -EngineRoot pointing at the UE_5.4 installation, for example:
  .\Scripts\BuildHellwake.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.4'
"@
}

$ResolvedEngineRoot = Resolve-UnrealRoot -ExplicitRoot $EngineRoot
$GenerateProjectFiles = Join-Path $ResolvedEngineRoot 'Engine\Build\BatchFiles\GenerateProjectFiles.bat'
$BuildBat = Join-Path $ResolvedEngineRoot 'Engine\Build\BatchFiles\Build.bat'
$EditorExe = Join-Path $ResolvedEngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'

Write-Host "Hellwake project : $ProjectFile"
Write-Host "Unreal Engine    : $ResolvedEngineRoot"
Write-Host "Configuration    : $Configuration"
Write-Host "Build log        : $BuildLog"

if (-not $SkipProjectFiles) {
    Write-Host "`n=== Generating project files ==="
    & $GenerateProjectFiles -project="$ProjectFile" -game -engine 2>&1 | Tee-Object -FilePath $BuildLog
    $GenerateExit = $LASTEXITCODE
    if ($GenerateExit -ne 0) {
        throw "GenerateProjectFiles failed with exit code $GenerateExit. Log: $BuildLog"
    }
}

Write-Host "`n=== Building HellwakeEditor Win64 $Configuration ==="
& $BuildBat HellwakeEditor Win64 $Configuration $ProjectFile -WaitMutex -NoHotReloadFromIDE 2>&1 | Tee-Object -FilePath $BuildLog -Append
$BuildExit = $LASTEXITCODE

if ($BuildExit -ne 0) {
    Write-Error "HellwakeEditor build failed with exit code $BuildExit. Log: $BuildLog"
    exit $BuildExit
}

Write-Host "`nHELLWAKEEDITOR BUILD SUCCEEDED" -ForegroundColor Green
Write-Host "Log: $BuildLog"

if ($LaunchEditor) {
    Write-Host "`n=== Launching Unreal Editor ==="
    Start-Process -FilePath $EditorExe -ArgumentList @("`"$ProjectFile`"") -WorkingDirectory $ProjectRoot
}

exit 0
