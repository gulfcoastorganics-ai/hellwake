[CmdletBinding()]
param(
    [string]$EngineRoot,
    [switch]$SkipBuild,
    [int]$ResX = 1600,
    [int]$ResY = 900
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'Hellwake.uproject'
$BuildScript = Join-Path $PSScriptRoot 'BuildHellwake.ps1'

function Test-UnrealRoot([string]$Root) {
    if ([string]::IsNullOrWhiteSpace($Root)) { return $false }
    return (Test-Path (Join-Path $Root 'Engine\Binaries\Win64\UnrealEditor.exe')) -and
           (Test-Path (Join-Path $Root 'Engine\Build\BatchFiles\Build.bat'))
}

function Resolve-UnrealRoot([string]$ExplicitRoot) {
    if (Test-UnrealRoot $ExplicitRoot) {
        return (Resolve-Path $ExplicitRoot).Path
    }

    $Candidates = @()
    foreach ($RegistryPath in @(
        'HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.4',
        'HKLM:\SOFTWARE\WOW6432Node\EpicGames\Unreal Engine\5.4'
    )) {
        try {
            $InstallLocation = (Get-ItemProperty -Path $RegistryPath -Name InstalledDirectory -ErrorAction Stop).InstalledDirectory
            if ($InstallLocation) { $Candidates += $InstallLocation }
        } catch { }
    }

    $Candidates += @(
        'C:\Program Files\Epic Games\UE_5.4',
        'C:\Epic Games\UE_5.4',
        'D:\Epic Games\UE_5.4',
        'D:\UE_5.4'
    )

    foreach ($Candidate in $Candidates | Select-Object -Unique) {
        if (Test-UnrealRoot $Candidate) {
            return (Resolve-Path $Candidate).Path
        }
    }

    throw @"
Unreal Engine 5.4 was not found automatically.
Run again with -EngineRoot, for example:
  .\Scripts\RunHellwakeNative.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.4'
"@
}

if (-not (Test-Path $ProjectFile)) {
    throw "Hellwake.uproject not found at $ProjectFile"
}

$ResolvedEngineRoot = Resolve-UnrealRoot $EngineRoot
$EditorExe = Join-Path $ResolvedEngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'

if (-not $SkipBuild) {
    Write-Host "=== Building HellwakeEditor first ===" -ForegroundColor Cyan
    & $BuildScript -EngineRoot $ResolvedEngineRoot -Configuration Development
    if ($LASTEXITCODE -ne 0) {
        throw "HellwakeEditor build failed. See Saved\BuildLogs."
    }
}

Write-Host "=== Launching native Hellwake slice ===" -ForegroundColor Green
Write-Host "Engine : $ResolvedEngineRoot"
Write-Host "Project: $ProjectFile"
Write-Host "Map    : /Engine/Maps/Entry (runtime C++ arena is spawned by AHellwakeGameMode)"

$Arguments = @(
    "`"$ProjectFile`"",
    '/Engine/Maps/Entry',
    '-game',
    '-windowed',
    "-ResX=$ResX",
    "-ResY=$ResY",
    '-log'
)

Start-Process -FilePath $EditorExe -ArgumentList $Arguments -WorkingDirectory $ProjectRoot
