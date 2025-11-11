# Screen Recorder V2

A clean, well-architected command-line screen recording tool for Windows using the Windows.Graphics.Capture API.

## Overview

Screen Recorder V2 is a complete rewrite of the original screen recorder tool with the following improvements:

- **Clean Architecture**: Proper separation of concerns with distinct modules for CLI parsing, IPC, screen capture, and frame buffering
- **Comprehensive Error Handling**: All error paths are properly handled with informative error messages
- **Verbose Logging**: Built-in verbose mode for debugging runtime issues
- **Modern C++ Best Practices**: Uses RAII, smart pointers, and modern C++20 features
- **Intuitive Command-Line Interface**: Clear, consistent commands with comprehensive help system
- **Thread-Safe Design**: Proper synchronization for concurrent operations
- **Robust IPC**: Clean named pipe implementation with proper error handling

## Architecture

### Module Structure

```
ScreenRecorderV2/
├── include/
│   ├── pch.h                    # Precompiled header
│   ├── Logger.h                 # Thread-safe logging with verbose mode
│   ├── CommandLineParser.h      # CLI argument parsing and validation
│   ├── IPC.h                    # Named pipe client/server communication
│   ├── MonitorInfo.h            # Monitor enumeration utilities
│   ├── FrameBuffer.h            # Circular frame buffer management
│   ├── ScreenCapture.h          # Screen capture engine
│   └── RecordingSession.h       # Recording session and process management
└── src/
    ├── pch.cpp                  # Precompiled header implementation
    ├── Logger.cpp               # Logger implementation
    ├── CommandLineParser.cpp    # CLI parser implementation
    ├── IPC.cpp                  # IPC implementation
    ├── MonitorInfo.cpp          # Monitor utilities implementation
    ├── FrameBuffer.cpp          # Frame buffer implementation
    ├── ScreenCapture.cpp        # Screen capture implementation
    ├── RecordingSession.cpp     # Session management implementation
    └── main.cpp                 # Application entry point
```

### Key Components

1. **Logger**: Thread-safe logging system with timestamp formatting and verbose mode support
2. **CommandLineParser**: Comprehensive argument parsing with validation and help text
3. **IPC Module**: RAII-based named pipe implementation for client-server communication
4. **MonitorInfo**: Utilities for enumerating and selecting display monitors
5. **FrameBuffer**: Thread-safe circular buffer with memory management (MB or frame count based)
6. **ScreenCapture**: Windows.Graphics.Capture wrapper with frame timing control
7. **RecordingSession**: High-level recording session management
8. **Client/Server Architecture**: Separate processes for command handling and recording

## Usage

### Starting a Recording

```bash
# Start with default settings (1 FPS, monitor 0, 100 MB buffer)
screenrecorder start

# Start with custom framerate
screenrecorder start --framerate 30

# Start with specific monitor and buffer size
screenrecorder start --monitor 1 --buffer 500 mb

# Start with frame-count based buffer
screenrecorder start --buffer 1000 frames

# Start with verbose logging
screenrecorder start --framerate 10 --verbose
```

### Stopping and Saving

```bash
# Stop recording and save frames to a directory
screenrecorder stop "C:\recordings\session1"

# Stop with verbose logging
screenrecorder stop "D:\output" --verbose
```

### Canceling a Recording

```bash
# Cancel recording without saving frames
screenrecorder cancel
```

### Getting Help

```bash
# General help
screenrecorder help

# Help for specific commands
screenrecorder help start
screenrecorder help stop
screenrecorder help cancel
```

## Command-Line Options

### Global Options

- `--verbose`, `-v`: Enable verbose logging for debugging

### Start Command Options

- `--framerate`, `-f <rate>`: Frames per second (1-60, default: 1)
- `--monitor`, `-m <index>`: Monitor index to record (0-based, -1 for all monitors, default: 0)
- `--buffer`, `-b <size> [unit]`: Buffer size with optional unit
  - Units: `mb` (megabytes, default) or `frames` (frame count)
  - Default: 100 MB

### Stop Command

- Takes a single argument: output directory path (must exist)

### Cancel Command

- No additional arguments required

## Technical Details

### Process Architecture

The tool uses a client-server architecture:

- **Client Process**: Handles command-line input and communicates with server via named pipes
- **Server Process**: Runs in the background, performs screen capture, and manages frame buffer

When you start a recording:
1. Client checks if server is already running
2. If not, client spawns a new server process
3. Client sends start command to server via named pipe
4. Server begins capturing frames and buffering them
5. Client exits, leaving server running in background

When you stop/cancel:
1. Client connects to running server
2. Client sends stop/cancel command
3. Server saves frames (if stop) or discards them (if cancel)
4. Server shuts down

### Frame Buffer Management

The circular frame buffer supports two capacity modes:

1. **Memory-based** (default): Keeps frames until memory limit is reached, then discards oldest
2. **Frame-count based**: Keeps a fixed number of frames, discarding oldest when full

Frames are stored as Direct3D11 textures in memory and saved as JPEG images on stop.

### Error Handling

All error conditions are properly handled:

- Invalid command-line arguments with helpful error messages
- Connection failures with timeout and retry logic
- File I/O errors with detailed diagnostics
- WinRT/COM errors with human-readable messages
- Resource cleanup via RAII (no memory leaks)

### Logging

Two logging levels:

- **Normal**: Shows important operations and errors
- **Verbose** (`--verbose` flag): Shows detailed debugging information including:
  - IPC message traffic
  - Frame capture timing
  - Memory usage
  - Monitor enumeration
  - All internal operations

Logs include microsecond-precision timestamps for debugging timing issues.

## Building

### Prerequisites

- Visual Studio 2022 or later
- Windows 10 SDK (10.0.19041.0 or later)
- C++20 compiler support

### NuGet Dependencies

The project uses the following NuGet packages (automatically restored):

- `Microsoft.Windows.CppWinRT` (2.0.220531.1)
- `Microsoft.Windows.ImplementationLibrary` (1.0.230411.1)
- `robmikh.common` (0.0.22-beta)

### Build Steps

1. Open `ScreenRecorderV2.sln` in Visual Studio
2. Select your desired configuration (Debug/Release) and platform (x64/ARM64)
3. Build the solution (Ctrl+Shift+B)

The executable will be output to the appropriate build directory.

## Comparison with Original Version

### Improvements

1. **Modular Design**: Code is organized into logical, reusable modules
2. **Error Handling**: All error paths properly handled with informative messages
3. **Logging System**: Built-in verbose logging for debugging
4. **Better CLI**: More intuitive commands with comprehensive help
5. **Modern C++**: Uses C++20 features, RAII, smart pointers
6. **Documentation**: Extensive inline comments explaining functionality
7. **Type Safety**: Strong typing throughout, minimal use of void*/raw pointers
8. **Resource Management**: Proper cleanup via RAII, no manual memory management
9. **Thread Safety**: Proper synchronization primitives where needed

### Maintained Features

- Windows.Graphics.Capture API for screen recording
- Named pipe IPC for client-server communication
- Circular frame buffer with configurable capacity
- JPEG output format
- Multi-monitor support

## Future Enhancements

Possible improvements for future versions:

- PNG/BMP output format options
- Video encoding (MP4/AVI output)
- Region/window capture (not just monitors)
- Real-time preview window
- Configurable JPEG quality
- Scheduled recording start/stop
- Performance metrics output

## License

This project follows the same license as the parent repository.

## Contributing

When contributing to this codebase, please follow these guidelines:

1. Maintain the modular architecture
2. Add appropriate error handling for all operations
3. Include verbose logging for debugging
4. Document all public APIs with XML comments
5. Follow existing code style and conventions
6. Test error paths, not just happy paths
