# Screen Recorder V2 - Quick Start Guide

## What's Been Created

A brand new, professionally architected screen recording tool in `D:\screenrecorder\ScreenRecorderV2\` with:

✅ **Clean Architecture** - Modular design with proper separation of concerns
✅ **Comprehensive Error Handling** - All error paths covered with helpful messages  
✅ **Verbose Logging** - Built-in debug logging with `--verbose` flag
✅ **Modern C++20** - RAII, smart pointers, best practices throughout
✅ **Intuitive CLI** - User-friendly commands with built-in help
✅ **Full Documentation** - Comments throughout the codebase

## Project Structure

```
ScreenRecorderV2/
├── ScreenRecorderV2.sln              # Visual Studio solution
├── README.md                          # Full documentation
└── ScreenRecorderV2/
    ├── ScreenRecorderV2.vcxproj      # Visual Studio project
    ├── packages.config                # NuGet dependencies
    ├── include/                       # Header files
    │   ├── pch.h                     # Precompiled header
    │   ├── Logger.h                  # Logging system
    │   ├── CommandLineParser.h       # CLI parsing
    │   ├── IPC.h                     # Inter-process communication
    │   ├── MonitorInfo.h             # Monitor enumeration
    │   ├── FrameBuffer.h             # Frame buffering
    │   ├── ScreenCapture.h           # Screen capture engine
    │   └── RecordingSession.h        # Session management
    └── src/                          # Implementation files
        ├── pch.cpp
        ├── Logger.cpp
        ├── CommandLineParser.cpp
        ├── IPC.cpp
        ├── MonitorInfo.cpp
        ├── FrameBuffer.cpp
        ├── ScreenCapture.cpp
        ├── RecordingSession.cpp
        └── main.cpp                  # Entry point
```

## Key Improvements Over Original

### 1. **Modular Architecture**
- **Logger**: Thread-safe logging with timestamps and verbose mode
- **CommandLineParser**: Robust argument parsing with validation
- **IPC**: Clean named pipe abstraction with RAII
- **FrameBuffer**: Memory-managed circular buffer
- **ScreenCapture**: Decoupled capture logic
- **RecordingSession**: High-level session management

### 2. **Error Handling**
- Every operation has proper error handling
- Informative error messages
- No silent failures
- Graceful cleanup on errors

### 3. **Logging System**
```cpp
// Normal mode: Shows important operations
screenrecorder start

// Verbose mode: Shows all internal operations
screenrecorder start --verbose
```

### 4. **Better Command-Line Interface**
```bash
# Old style (still supported):
screenrecorder -start -framerate 30

# New style (cleaner):
screenrecorder start --framerate 30 --buffer 500 mb --verbose
```

### 5. **Code Quality**
- Comprehensive XML documentation comments
- RAII for all resource management
- Smart pointers (no manual delete)
- Const correctness
- Thread-safe where needed
- Modern C++20 features

## How to Build

1. **Open in Visual Studio**
   - Open `ScreenRecorderV2.sln`
   - Visual Studio will automatically restore NuGet packages

2. **Select Configuration**
   - Debug or Release
   - x64 or ARM64

3. **Build**
   - Press `Ctrl+Shift+B` or Build → Build Solution
   - Output will be in the appropriate build directory

## Usage Examples

### Basic Recording
```bash
# Start recording with defaults
screenrecorder start

# Stop and save
screenrecorder stop "C:\recordings"

# Or cancel without saving
screenrecorder cancel
```

### Advanced Recording
```bash
# High framerate with large buffer
screenrecorder start --framerate 30 --buffer 1000 mb --verbose

# Record specific monitor
screenrecorder start --monitor 1 --framerate 10

# Buffer by frame count instead of memory
screenrecorder start --buffer 5000 frames
```

### Getting Help
```bash
screenrecorder help
screenrecorder help start
screenrecorder help stop
```

## Architecture Highlights

### Client-Server Design
- **Client**: Handles CLI and sends commands
- **Server**: Runs in background, performs capture
- Communication via named pipes (IPC)

### Process Flow
```
1. User runs: screenrecorder start --framerate 30
2. Client parses arguments
3. Client spawns server process
4. Client sends start command to server via named pipe
5. Server begins capturing frames
6. Client exits (server continues in background)

Later...

7. User runs: screenrecorder stop "C:\output"
8. Client sends stop command to server
9. Server saves all buffered frames
10. Server shuts down
```

### Frame Buffer
- Circular buffer with configurable capacity
- Two modes: memory-based (MB) or frame-count based
- Thread-safe operations
- Automatic oldest-frame eviction when full

## Code Organization

### Headers (`include/`)
All classes have:
- XML documentation comments
- Clear public/private separation  
- RAII design (constructors/destructors handle resources)
- No raw pointers in public APIs

### Implementation (`src/`)
All functions have:
- Verbose logging at key points
- Comprehensive error handling
- Clear variable names
- Logical flow

### Example: Adding a New Feature

To add a new command, you'd:

1. Update `CommandType` enum in `CommandLineParser.h`
2. Add parsing logic in `CommandLineParser.cpp`
3. Add IPC message type in `IPC.h` if needed
4. Add handler in `RecordingSession.cpp`
5. Wire up in `main.cpp`

Each module is independent, making changes easy and safe.

## Dependencies

The project uses the same NuGet packages as the original:
- **Microsoft.Windows.CppWinRT**: WinRT support
- **Microsoft.Windows.ImplementationLibrary**: WIL utilities
- **robmikh.common**: Screen capture helpers

These are automatically restored by Visual Studio.

## Next Steps

1. **Build the Project**
   - Open `ScreenRecorderV2.sln`
   - Build and test

2. **Test the Tool**
   ```bash
   # Test basic recording
   screenrecorder start --verbose
   # Wait a few seconds
   screenrecorder stop "C:\temp\test"
   # Check that frames were saved
   ```

3. **Explore the Code**
   - Start with `main.cpp` to see the flow
   - Read `CommandLineParser.cpp` to understand argument handling
   - Check `RecordingSession.cpp` for the core logic
   - Look at `Logger.cpp` to see the logging system

4. **Customize as Needed**
   - All modules are well-documented
   - Easy to modify or extend
   - Each component is independent

## Troubleshooting

If you encounter build issues:

1. **NuGet Package Restore Failed**
   - Right-click solution → Restore NuGet Packages
   - Or: Tools → NuGet Package Manager → Package Manager Console
   - Run: `Update-Package -Reinstall`

2. **Windows SDK Version**
   - Project targets Windows 10 SDK (any version)
   - If you have a different SDK, Visual Studio will prompt to retarget

3. **C++20 Not Available**
   - Ensure Visual Studio 2022 or later
   - Update to latest VS version if needed

## Questions?

Refer to:
- `README.md` - Full documentation
- Code comments - Extensive inline documentation
- Original code - In `D:\screenrecorder\screenrecorder\ScreenRecorder\`

Enjoy your clean, maintainable screen recorder! 🎉
