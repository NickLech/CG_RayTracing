param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release"
)

$root = "C:\Users\Utente\Desktop\CG_RayTracing"
$build = "$root\build"
$exeDir = "$build\bin\$Config"
$exe = "$exeDir\CG_Raytracing.exe"

if (-not (Test-Path $build)) {
    New-Item -ItemType Directory -Path $build | Out-Null
    cmake -S $root -B $build
}

cmake --build $build --config $Config

if ($LASTEXITCODE -eq 0) {
    Set-Location $exeDir
    & $exe
} else {
    Write-Host "Build fallita." -ForegroundColor Red
}