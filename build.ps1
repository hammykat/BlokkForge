# Configures and builds Blokk using CMakePresets.json
# Usage: ./build.ps1 [debug|release] [-Clean]

param(
    [ValidateSet("debug", "release")]
    [string]$Preset = "debug",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Make sure the MSYS2 UCRT64 toolchain (gcc, g++, ninja) is on PATH
$MinGWBin = if ($env:MSYS2_ROOT) { Join-Path $env:MSYS2_ROOT "ucrt64/bin" } else { "C:/msys64/ucrt64/bin" }
if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
    if (-not (Test-Path $MinGWBin)) {
        throw "g++ not found. Install MSYS2 UCRT64 (pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-ninja) or set MSYS2_ROOT."
    }
    # Appended, not prepended, so MSYS2's own cmake.exe doesn't shadow the system one
    $env:PATH = "$env:PATH;$MinGWBin"
}

$BuildDir = Join-Path $PSScriptRoot "build/$Preset"
if ($Clean -and (Test-Path $BuildDir)) {
    Remove-Item -Recurse -Force $BuildDir
}

Push-Location $PSScriptRoot
try {
    cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "Configure failed" }

    cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
}
finally {
    Pop-Location
}
