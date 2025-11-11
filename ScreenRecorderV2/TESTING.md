# Testing Guide for Screen Recorder V2

## Overview

This guide covers how to build, run, and write tests for the Screen Recorder V2 project. The test suite uses **Google Test (gtest)**, the industry-standard C++ testing framework.

## Table of Contents

1. [What's Been Tested](#whats-been-tested)
2. [Project Structure](#project-structure)
3. [Building the Tests](#building-the-tests)
4. [Running the Tests](#running-the-tests)
5. [Understanding the Test Suite](#understanding-the-test-suite)
6. [Writing New Tests](#writing-new-tests)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)

---

## What's Been Tested

The test suite provides comprehensive coverage of key components:

### ✅ **CommandLineParser** (50+ tests)
- Basic command parsing (start, stop, cancel, help, server)
- Parameter parsing (framerate, monitor, buffer)
- Verbose flag handling
- Validation (boundary checks, error cases)
- Edge cases and error handling

### ✅ **FrameBuffer** (25+ tests)
- Frame addition and capacity management
- Memory-based and frame-based capacity modes
- Thread safety with concurrent access
- Memory usage calculation
- Clear operations
- Edge cases (small buffers, large frame counts)

### ✅ **Logger** (35+ tests)
- Logging levels (Error, Info, Verbose)
- Verbose mode toggling
- Thread-safe concurrent logging
- Message formatting and timestamps
- State persistence
- Real-world scenario testing

### ✅ **TestUtilities**
- Mock D3D11 texture creation
- Direct3D device initialization
- Temporary directory management
- Helper functions for testing

**Total: 110+ comprehensive unit tests**

---

## Project Structure

```
ScreenRecorderV2/
├── ScreenRecorderV2/                 # Main application
│   ├── include/                      # Headers
│   └── src/                          # Implementation
│
└── ScreenRecorderTests/              # Test project ⭐ NEW
    ├── ScreenRecorderTests.vcxproj   # Visual Studio project
    ├── packages.config               # NuGet dependencies
    │
    ├── pch.h / pch.cpp              # Precompiled headers
    ├── TestUtilities.h / .cpp       # Test helper functions
    │
    ├── CommandLineParserTests.cpp   # 50+ parser tests
    ├── FrameBufferTests.cpp         # 25+ buffer tests
    └── LoggerTests.cpp              # 35+ logger tests
```

---

## Building the Tests

### Option 1: Visual Studio (Recommended)

1. **Open the Solution**
   ```
   Open ScreenRecorderV2.sln in Visual Studio 2022
   ```

2. **Restore NuGet Packages**
   - Right-click on the solution → **Restore NuGet Packages**
   - This installs Google Test and other dependencies automatically

3. **Select Test Project**
   - In Solution Explorer, locate **ScreenRecorderTests**
   - Right-click → **Set as Startup Project** (optional, for easy debugging)

4. **Build**
   - Select **Debug | x64** configuration
   - Press **Ctrl+Shift+B** or **Build → Build Solution**
   - Output: `ScreenRecorderV2\x64\Debug\ScreenRecorderTests.exe`

### Option 2: Command Line (MSBuild)

```powershell
# From the ScreenRecorderV2 directory
cd D:\screenrecorder\ScreenRecorderV2

# Restore NuGet packages
nuget restore ScreenRecorderV2.sln

# Build the test project
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64
```

---

## Running the Tests

### Option 1: Visual Studio Test Explorer

1. **Open Test Explorer**
   - **Test → Test Explorer** (or press **Ctrl+E, T**)

2. **Discover Tests**
   - After building, tests should appear automatically
   - If not, click the **refresh** icon

3. **Run All Tests**
   - Click **Run All** in Test Explorer
   - View results in real-time with pass/fail indicators

4. **Run Specific Tests**
   - Right-click any test → **Run**
   - Run by test class, namespace, or individual test

5. **Debug Tests**
   - Right-click a test → **Debug Selected Tests**
   - Set breakpoints in test code or source code

### Option 2: Command Line

```powershell
# Run all tests
cd D:\screenrecorder\ScreenRecorderV2\x64\Debug
.\ScreenRecorderTests.exe

# Run with verbose output
.\ScreenRecorderTests.exe --gtest_verbose

# Run specific tests by filter
.\ScreenRecorderTests.exe --gtest_filter=CommandLineParserTest.*

# Run only FrameBuffer tests
.\ScreenRecorderTests.exe --gtest_filter=FrameBufferTest.*

# List all tests without running
.\ScreenRecorderTests.exe --gtest_list_tests
```

### Option 3: Visual Studio Code (with C++ extension)

1. Install the **C++ TestMate** extension
2. Configure test discovery to find `ScreenRecorderTests.exe`
3. Run tests from the Testing view

---

## Understanding the Test Suite

### Test Organization

Each test file follows a consistent pattern:

```cpp
// Test Fixture - Shared setup/teardown
class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Runs before each test
    }

    void TearDown() override {
        // Runs after each test
    }
};

// Individual Test
TEST_F(MyComponentTest, TestName) {
    // Arrange - Set up test data
    // Act - Call the function
    // Assert - Verify results
    EXPECT_EQ(actual, expected);
}
```

### Common Assertions

```cpp
// Equality checks
EXPECT_EQ(actual, expected);     // Equal
EXPECT_NE(actual, expected);     // Not equal
EXPECT_LT(actual, expected);     // Less than
EXPECT_LE(actual, expected);     // Less or equal
EXPECT_GT(actual, expected);     // Greater than
EXPECT_GE(actual, expected);     // Greater or equal

// Boolean checks
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Exception checks
EXPECT_THROW(code, ExceptionType);
EXPECT_NO_THROW(code);

// String checks
EXPECT_STREQ(str1, str2);
```

### Test Categories

Tests are organized by purpose:

1. **Basic Functionality** - Core features work correctly
2. **Validation** - Input validation and error handling
3. **Boundary Tests** - Edge cases and limits
4. **Thread Safety** - Concurrent access scenarios
5. **Integration** - Real-world usage patterns

---

## Writing New Tests

### Step 1: Choose the Right Test File

- **CommandLineParserTests.cpp** - Argument parsing, commands
- **FrameBufferTests.cpp** - Frame storage, capacity management
- **LoggerTests.cpp** - Logging, verbose mode
- **New component?** - Create a new `ComponentTests.cpp` file

### Step 2: Add a Test

```cpp
TEST_F(YourComponentTest, DescriptiveTestName) {
    // Arrange - Set up
    YourComponent component;
    int expectedValue = 42;

    // Act - Execute
    int actualValue = component.DoSomething();

    // Assert - Verify
    EXPECT_EQ(actualValue, expectedValue);
}
```

### Step 3: Update Project File

If you created a new test file, add it to `ScreenRecorderTests.vcxproj`:

```xml
<ItemGroup>
  <ClCompile Include="YourNewTests.cpp" />
</ItemGroup>
```

### Step 4: Build and Run

```powershell
# Build
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64

# Run your new tests
.\x64\Debug\ScreenRecorderTests.exe --gtest_filter=YourComponentTest.*
```

---

## Best Practices

### ✅ DO

1. **Use descriptive test names**
   ```cpp
   TEST_F(ParserTest, StartWithInvalidFramerateTooHigh_ThrowsException)
   // Clear what's being tested and expected result
   ```

2. **Follow Arrange-Act-Assert pattern**
   ```cpp
   // Arrange - Set up test data
   auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "61"});
   
   // Act & Assert - Execute and verify
   EXPECT_THROW(CommandLineParser::Parse(...), std::invalid_argument);
   ```

3. **Test one thing per test**
   - Each test should verify a single behavior
   - Makes failures easier to diagnose

4. **Use test fixtures for shared setup**
   ```cpp
   class MyTest : public ::testing::Test {
   protected:
       void SetUp() override { /* Common setup */ }
       
       // Shared helper methods
       std::vector<char*> CreateArgv(...);
   };
   ```

5. **Clean up resources**
   ```cpp
   void TearDown() override {
       TestUtilities::CleanupTempDirectory(tempPath);
   }
   ```

### ❌ DON'T

1. **Don't write tests that depend on other tests**
   - Each test should be independent
   - Order of execution is not guaranteed

2. **Don't use hard-coded paths**
   ```cpp
   // Bad
   std::string path = "C:\\specific\\path";
   
   // Good
   auto path = TestUtilities::CreateTempDirectory();
   ```

3. **Don't test too much in one test**
   ```cpp
   // Bad - tests multiple behaviors
   TEST_F(ParserTest, EverythingAboutStart) { /* 50 lines */ }
   
   // Good - focused tests
   TEST_F(ParserTest, StartWithFramerate_ParsedCorrectly) { /* 5 lines */ }
   TEST_F(ParserTest, StartWithMonitor_ParsedCorrectly) { /* 5 lines */ }
   ```

4. **Don't ignore test failures**
   - All tests should pass before committing code
   - Fix broken tests immediately

---

## Test Coverage Highlights

### CommandLineParser Coverage
```
✓ All command types (start, stop, cancel, help, server)
✓ All parameters (framerate, monitor, buffer)
✓ Validation (range checks, required fields)
✓ Error cases (missing args, invalid values)
✓ Legacy syntax support (-start vs start)
✓ Verbose flag in any position
```

### FrameBuffer Coverage
```
✓ Memory-based capacity (MB)
✓ Frame-count capacity
✓ Automatic oldest-frame eviction
✓ Thread-safe concurrent access
✓ Memory usage tracking
✓ Clear operations
✓ Edge cases (1 MB buffer, 100k frames)
```

### Logger Coverage
```
✓ All logging levels (Error, Info, Verbose)
✓ Verbose mode on/off
✓ Thread-safe multi-threaded logging
✓ Message formatting with timestamps
✓ State persistence
✓ Real-world scenarios
```

---

## Troubleshooting

### Issue: Tests don't appear in Test Explorer

**Solution:**
1. Rebuild the test project
2. Close and reopen Test Explorer
3. Ensure Google Test package is installed: `Tools → NuGet Package Manager → Manage NuGet Packages for Solution`
4. Check that tests have the `TEST_F` or `TEST` macro

### Issue: "Cannot find Google Test"

**Solution:**
```powershell
# Reinstall Google Test package
nuget restore ScreenRecorderV2.sln
```

Or in Visual Studio:
- Right-click solution → **Restore NuGet Packages**

### Issue: Tests crash on startup

**Possible causes:**
1. **Missing D3D11**: Tests need a graphics device
   - Run on a machine with graphics drivers
   - Tests will fallback to WARP (software rendering) if needed

2. **WinRT initialization**: Some tests require COM/WinRT
   - Check `main()` has `winrt::init_apartment()`

### Issue: "Access Denied" when creating temp directories

**Solution:**
```cpp
// Use TestUtilities::CreateTempDirectory() instead of hard-coded paths
auto tempDir = TestUtilities::CreateTempDirectory();
```

### Issue: Build errors about missing headers

**Solution:**
- Ensure precompiled headers are set correctly:
  ```
  pch.cpp → Precompiled Header: Create (/Yc)
  Other .cpp files → Precompiled Header: Use (/Yu)
  ```

### Issue: Tests pass locally but fail on CI/CD

**Common causes:**
- Hard-coded paths (use temp directories)
- Timing-dependent tests (add proper synchronization)
- Tests that require user interaction (mock it)
- Tests that depend on external resources (mock it)

---

## Advanced Testing Scenarios

### Testing Multithreaded Code

```cpp
TEST_F(FrameBufferTest, AddFrame_MultithreadedAccess_ThreadSafe) {
    FrameBuffer buffer(100, true);
    std::vector<std::thread> threads;
    
    // Spawn multiple threads
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&buffer, t]() {
            for (int i = 0; i < 25; ++i) {
                auto texture = TestUtilities::CreateMockTexture(800, 600);
                buffer.AddFrame(texture, "frame.jpg");
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no crashes and reasonable state
    EXPECT_GT(buffer.GetFrameCount(), 0);
}
```

### Testing Exception Handling

```cpp
TEST_F(ParserTest, InvalidCommand_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "invalid"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(argv.size(), argv.data()),
        std::invalid_argument
    );
}
```

### Capturing Output

```cpp
TEST_F(LoggerTest, Info_LogsCorrectly) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info("Test message");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test message"), std::string::npos);
}
```

---

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Run Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup MSBuild
        uses: microsoft/setup-msbuild@v1
      
      - name: Restore NuGet
        run: nuget restore ScreenRecorderV2.sln
      
      - name: Build Tests
        run: msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Release /p:Platform=x64
      
      - name: Run Tests
        run: .\x64\Release\ScreenRecorderTests.exe --gtest_output=xml:test_results.xml
      
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: test_results.xml
```

---

## Summary

You now have a complete testing infrastructure for Screen Recorder V2:

- ✅ **110+ comprehensive unit tests** covering core components
- ✅ **Google Test framework** - industry standard
- ✅ **Test utilities** for mocking and helpers
- ✅ **Thread safety testing** for concurrent operations
- ✅ **Easy to extend** - add new tests following existing patterns
- ✅ **CI/CD ready** - automated testing support

### Quick Reference Commands

```powershell
# Build tests
msbuild ScreenRecorderTests\ScreenRecorderTests.vcxproj /p:Configuration=Debug /p:Platform=x64

# Run all tests
.\x64\Debug\ScreenRecorderTests.exe

# Run specific test class
.\x64\Debug\ScreenRecorderTests.exe --gtest_filter=CommandLineParserTest.*

# Run with verbose output
.\x64\Debug\ScreenRecorderTests.exe --gtest_verbose

# List all tests
.\x64\Debug\ScreenRecorderTests.exe --gtest_list_tests
```

### Next Steps

1. **Build and run the tests** to ensure everything works
2. **Explore the test files** to understand the patterns
3. **Add tests for new features** as you develop them
4. **Run tests before committing** to catch issues early

Happy testing! 🎉
