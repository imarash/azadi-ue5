<#
.SYNOPSIS
    Cook, stage, and package AZADI for Windows (Win64) and produce both a
    portable .zip (containing Azadi.exe) and a Windows installer (.exe).

.DESCRIPTION
    Wraps Unreal Automation Tool (RunUAT BuildCookRun). Designed to run on a
    self-hosted GitHub runner that has UE 5.7 installed, but also works locally.

    Outputs land in <Output>:
      - Azadi-Windows-<Version>.zip      (portable, unzip-and-run)
      - AzadiSetup-<Version>.exe         (Inno Setup installer)

.NOTES
    Requires:
      - Unreal Engine 5.7 (set $env:UE_ROOT or pass -UeRoot)
      - Inno Setup 6 (ISCC.exe on PATH or installed in the default location)
        for the installer step. Skipped with a warning if not found.
#>
[CmdletBinding()]
param(
    [string]$UeRoot = $env:UE_ROOT,
    [string]$Configuration = "Shipping",
    [string]$Version = $env:AZADI_VERSION,
    [string]$Output = "$PSScriptRoot/../../dist"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RepoRoot   = (Resolve-Path "$PSScriptRoot/../..").Path
$Project    = Join-Path $RepoRoot "azadi.uproject"
$ArchiveDir = Join-Path $RepoRoot "Build"
$StageDir   = Join-Path $ArchiveDir "Windows"

function Resolve-Version {
    if ($Version) { return ($Version -replace '^v', '') }
    $ini = Join-Path $RepoRoot "Config/DefaultGame.ini"
    if (Test-Path $ini) {
        $m = Select-String -Path $ini -Pattern '^ProjectVersion=(.+)$'
        if ($m) { return $m.Matches[0].Groups[1].Value.Trim() }
    }
    return "0.0.0"
}

if (-not $UeRoot) {
    throw "UE_ROOT is not set. Point it at your UE 5.7 install (the folder containing Engine/)."
}
$RunUAT = Join-Path $UeRoot "Engine/Build/BatchFiles/RunUAT.bat"
if (-not (Test-Path $RunUAT)) {
    throw "RunUAT.bat not found at '$RunUAT'. Check UE_ROOT='$UeRoot'."
}

$Version = Resolve-Version
Write-Host "==> Packaging AZADI $Version (Win64 $Configuration)" -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path $Output | Out-Null

# --- Cook + stage + archive ------------------------------------------------
& $RunUAT BuildCookRun `
    "-project=$Project" `
    -noP4 -utf8output `
    -platform=Win64 `
    "-clientconfig=$Configuration" `
    -cook -build -stage -pak -package -archive `
    "-archivedirectory=$ArchiveDir"
if ($LASTEXITCODE -ne 0) { throw "RunUAT BuildCookRun failed ($LASTEXITCODE)." }

if (-not (Test-Path $StageDir)) {
    throw "Staged build not found at '$StageDir'."
}

# --- Portable zip ----------------------------------------------------------
$Zip = Join-Path $Output "Azadi-Windows-$Version.zip"
if (Test-Path $Zip) { Remove-Item $Zip -Force }
Write-Host "==> Zipping portable build -> $Zip" -ForegroundColor Cyan
Compress-Archive -Path (Join-Path $StageDir '*') -DestinationPath $Zip -CompressionLevel Optimal

# --- Installer (Inno Setup) ------------------------------------------------
$Iscc = Get-Command ISCC.exe -ErrorAction SilentlyContinue
if (-not $Iscc) {
    foreach ($p in @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles}\Inno Setup 6\ISCC.exe")) {
        if ($p -and (Test-Path $p)) { $Iscc = $p; break }
    }
}
if (-not $Iscc) {
    Write-Warning "Inno Setup (ISCC.exe) not found - skipping installer. Install it to produce AzadiSetup-$Version.exe."
} else {
    $IsccPath = if ($Iscc -is [string]) { $Iscc } else { $Iscc.Source }
    $Iss = Join-Path $RepoRoot "tools/installer/azadi.iss"
    Write-Host "==> Building installer with $IsccPath" -ForegroundColor Cyan
    & $IsccPath `
        "/DAppVersion=$Version" `
        "/DStageDir=$StageDir" `
        "/DOutputDir=$Output" `
        $Iss
    if ($LASTEXITCODE -ne 0) { throw "Inno Setup compile failed ($LASTEXITCODE)." }
}

Write-Host "==> Done. Artifacts in $Output" -ForegroundColor Green
Get-ChildItem $Output | Format-Table Name, Length
