# Screen Recorder V2 Tests - Quick Start

## Run Tests in 3 Steps

### 1. Open and Build
```powershell
# Open solution in Visual Studio
start ScreenRecorderV2.sln

# Or build from command line
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### 2. Run Tests
```powershell
cd x64\Debug
.\ScreenRecorderTests.exe
```

### 3. View Results
```
Expected output:
[==========] Running 110 tests from 3 test suites...
[----------] 50 tests from CommandLineParserTest
[----------] 25 tests from FrameBufferTest  
[----------] 35 tests from LoggerTest
[==========] 110 tests from 3 test suites ran.
[  PASSED  ] 110 tests.
```

## What's Tested

- ✅ **CommandLineParser** - 50+ tests for argument parsing and validation
- ✅ **FrameBuffer** - 25+ tests for frame storage and capacity management
- ✅ **Logger** - 35+ tests for logging and thread safety

## Common Commands

```powershell
# Run only parser tests
.\ScreenRecorderTests.exe --gtest_filter=CommandLineParserTest.*

# Run only buffer tests  
.\ScreenRecorderTests.exe --gtest_filter=FrameBufferTest.*

# Run only logger tests
.\ScreenRecorderTests.exe --gtest_filter=LoggerTest.*

# List all tests
.\ScreenRecorderTests.exe --gtest_list_tests

# Verbose output
.\ScreenRecorderTests.exe --gtest_verbose
```

## Visual Studio Test Explorer

1. Open **Test → Test Explorer** (Ctrl+E, T)
2. Click **Run All** to execute tests
3. View results with pass/fail indicators
4. Right-click any test → **Debug** to troubleshoot

## Need Help?

See [TESTING.md](TESTING.md) for complete documentation including:
- Detailed test descriptions
- How to write new tests
- Troubleshooting guide
- Best practices
- CI/CD integration

## Test Coverage

| Component | Tests | Coverage |
|-----------|-------|----------|
| CommandLineParser | 50+ | All commands, parameters, validation, error cases |
| FrameBuffer | 25+ | Memory/frame capacity, thread safety, edge cases |
| Logger | 35+ | All log levels, verbose mode, thread safety |
| **Total** | **110+** | **Core functionality fully tested** |

---

🎉 **Ready to test!** Run `.\x64\Debug\ScreenRecorderTests.exe` to get started.
