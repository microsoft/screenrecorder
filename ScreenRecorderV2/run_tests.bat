@echo off
REM Quick test runner for Screen Recorder V2
REM Run this script to build and execute all tests

echo ========================================
echo Screen Recorder V2 - Test Runner
echo ========================================
echo.

REM Check if we're in the right directory
if not exist "ScreenRecorderV2.sln" (
    echo ERROR: Please run this script from the ScreenRecorderV2 directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

echo [1/3] Restoring NuGet packages...
nuget restore ScreenRecorderV2.sln
if errorlevel 1 (
    echo ERROR: Failed to restore NuGet packages
    echo Please ensure NuGet is installed and in PATH
    pause
    exit /b 1
)
echo.

echo [2/3] Building test project...
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)
echo.

echo [3/3] Running tests...
echo ========================================
x64\Debug\ScreenRecorderTests.exe %*
set TEST_RESULT=%ERRORLEVEL%
echo ========================================
echo.

if %TEST_RESULT% equ 0 (
    echo ✓ All tests passed!
) else (
    echo ✗ Some tests failed
)

echo.
echo Test run complete.
echo.
echo TIP: Run with additional arguments:
echo   run_tests.bat --gtest_filter=CommandLineParserTest.*
echo   run_tests.bat --gtest_verbose
echo   run_tests.bat --gtest_list_tests
echo.

pause
exit /b %TEST_RESULT%
