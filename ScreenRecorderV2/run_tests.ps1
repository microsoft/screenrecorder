# Quick test runner for Screen Recorder V2
# Run this script to build and execute all tests

Write-Host "========================================"
Write-Host "Screen Recorder V2 - Test Runner"
Write-Host "========================================"
Write-Host ""

# Check if we're in the right directory
if (-not (Test-Path "ScreenRecorderV2.sln")) {
    Write-Host "ERROR: Please run this script from the ScreenRecorderV2 directory" -ForegroundColor Red
    Write-Host "Current directory: $PWD"
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "[1/3] Restoring NuGet packages..." -ForegroundColor Cyan
nuget restore ScreenRecorderV2.sln
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to restore NuGet packages" -ForegroundColor Red
    Write-Host "Please ensure NuGet is installed and in PATH"
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host ""

Write-Host "[2/3] Building test project..." -ForegroundColor Cyan
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host ""

Write-Host "[3/3] Running tests..." -ForegroundColor Cyan
Write-Host "========================================"
& "x64\Debug\ScreenRecorderTests.exe" $args
$testResult = $LASTEXITCODE
Write-Host "========================================"
Write-Host ""

if ($testResult -eq 0) {
    Write-Host "✓ All tests passed!" -ForegroundColor Green
} else {
    Write-Host "✗ Some tests failed" -ForegroundColor Red
}

Write-Host ""
Write-Host "Test run complete."
Write-Host ""
Write-Host "TIP: Run with additional arguments:" -ForegroundColor Yellow
Write-Host "  .\run_tests.ps1 --gtest_filter=CommandLineParserTest.*"
Write-Host "  .\run_tests.ps1 --gtest_verbose"
Write-Host "  .\run_tests.ps1 --gtest_list_tests"
Write-Host ""

Read-Host "Press Enter to exit"
exit $testResult
