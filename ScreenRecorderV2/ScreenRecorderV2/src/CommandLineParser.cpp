#include "pch.h"
#include "CommandLineParser.h"
#include <filesystem>

namespace ScreenRecorder {

ParsedArguments CommandLineParser::Parse(int argc, char* argv[]) {
    ParsedArguments args;

    if (argc < 2) {
        throw std::invalid_argument("No command specified");
    }

    std::string command = argv[1];

    // Check for verbose flag anywhere in arguments
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--verbose" || std::string(argv[i]) == "-v") {
            args.verbose = true;
            break;
        }
    }

    // Parse the command
    if (command == "start" || command == "-start") {
        args.command = CommandType::Start;
        args.startParams = ParseStartArguments(argc, argv, 2);
        ValidateStartParameters(args.startParams);
    }
    else if (command == "stop" || command == "-stop") {
        args.command = CommandType::Stop;
        args.stopParams = ParseStopArguments(argc, argv, 2);
    }
    else if (command == "cancel" || command == "-cancel") {
        args.command = CommandType::Cancel;
    }
    else if (command == "-server") {
        // Internal command for server process
        args.command = CommandType::Server;
    }
    else if (command == "help" || command == "-help" || command == "--help") {
        args.command = CommandType::Help;
        if (argc > 2) {
            args.helpTopic = argv[2];
        }
    }
    else {
        args.command = CommandType::Unknown;
        throw std::invalid_argument("Unknown command: " + command);
    }

    return args;
}

StartParameters CommandLineParser::ParseStartArguments(int argc, char* argv[], int startIndex) {
    StartParameters params;
    int i = startIndex;

    while (i < argc) {
        std::string arg = argv[i];

        // Skip verbose flag (already handled)
        if (arg == "--verbose" || arg == "-v") {
            ++i;
            continue;
        }

        if (arg == "--framerate" || arg == "-f") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--framerate requires a value");
            }
            params.framerate = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "--monitor" || arg == "-m") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--monitor requires a value");
            }
            params.monitorIndex = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "--buffer" || arg == "-b") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--buffer requires a value");
            }
            params.bufferCapacity = std::stoi(argv[i + 1]);
            
            // Check for unit specifier
            if (i + 2 < argc) {
                std::string unit = argv[i + 2];
                if (unit == "frames" || unit == "f") {
                    params.capacityInMegabytes = false;
                    i += 3;
                }
                else if (unit == "mb" || unit == "MB") {
                    params.capacityInMegabytes = true;
                    i += 3;
                }
                else {
                    // Default to MB if no valid unit specified
                    params.capacityInMegabytes = true;
                    i += 2;
                }
            }
            else {
                // Default to MB
                params.capacityInMegabytes = true;
                i += 2;
            }
        }
        else {
            throw std::invalid_argument("Unknown start parameter: " + arg);
        }
    }

    return params;
}

StopParameters CommandLineParser::ParseStopArguments(int argc, char* argv[], int startIndex) {
    StopParameters params;
    int i = startIndex;

    // Find the output path (skip verbose flag if present)
    while (i < argc) {
        std::string arg = argv[i];
        
        if (arg == "--verbose" || arg == "-v") {
            ++i;
            continue;
        }

        if (params.outputPath.empty()) {
            params.outputPath = arg;
            ++i;
        }
        else {
            throw std::invalid_argument("Unexpected argument: " + arg);
        }
    }

    if (params.outputPath.empty()) {
        throw std::invalid_argument("Output path is required for stop command");
    }

    // Validate that the path exists and is a directory
    std::filesystem::path path(params.outputPath);
    if (!std::filesystem::exists(path)) {
        throw std::invalid_argument("Output path does not exist: " + params.outputPath);
    }
    if (!std::filesystem::is_directory(path)) {
        throw std::invalid_argument("Output path is not a directory: " + params.outputPath);
    }

    // Convert to absolute path (required by WinRT StorageFolder API)
    params.outputPath = std::filesystem::absolute(path).string();

    return params;
}

void CommandLineParser::ValidateStartParameters(const StartParameters& params) {
    if (params.framerate < 1 || params.framerate > 60) {
        throw std::invalid_argument("Framerate must be between 1 and 60");
    }

    if (params.monitorIndex < -1) {
        throw std::invalid_argument("Monitor index must be -1 (all monitors) or 0 or greater");
    }

    if (params.bufferCapacity < 1) {
        throw std::invalid_argument("Buffer capacity must be at least 1");
    }

    if (params.capacityInMegabytes && params.bufferCapacity > 10240) {
        throw std::invalid_argument("Buffer capacity cannot exceed 10240 MB (10 GB)");
    }

    if (!params.capacityInMegabytes && params.bufferCapacity > 100000) {
        throw std::invalid_argument("Buffer capacity cannot exceed 100000 frames");
    }
}

void CommandLineParser::ShowHelp() {
    std::cout << R"(
Screen Recorder V2 - Command-line screen recording tool

USAGE:
    screenrecorder <command> [options]

COMMANDS:
    start       Start a new recording session
    stop        Stop recording and save captured frames
    cancel      Cancel recording without saving frames
    help        Show help information

OPTIONS:
    --verbose, -v    Enable verbose logging

EXAMPLES:
    screenrecorder start --framerate 30 --buffer 500 mb
    screenrecorder stop "D:\recordings\session1"
    screenrecorder cancel

For detailed help on a specific command, use:
    screenrecorder help <command>
)";
}

void CommandLineParser::ShowStartHelp() {
    std::cout << R"(
START COMMAND - Start a new recording session

USAGE:
    screenrecorder start [options]

OPTIONS:
    --framerate, -f <rate>       Frames per second (1-60, default: 1)
    --monitor, -m <index>        Monitor to record (0-based, -1 for all, default: 0)
    --buffer, -b <size> [unit]   Buffer size (default: 100 mb)
                                 Units: mb (megabytes) or frames (frame count)
    --verbose, -v                Enable verbose logging

EXAMPLES:
    # Record at 30 FPS with 500 MB buffer
    screenrecorder start --framerate 30 --buffer 500 mb

    # Record monitor 1 at 10 FPS with buffer for 1000 frames
    screenrecorder start --framerate 10 --monitor 1 --buffer 1000 frames

    # Record all monitors with verbose logging
    screenrecorder start --monitor -1 --verbose

NOTES:
    - Only one recording session can be active at a time
    - Recording runs in a background process
    - Use 'stop' to save frames or 'cancel' to discard them
)";
}

void CommandLineParser::ShowStopHelp() {
    std::cout << R"(
STOP COMMAND - Stop recording and save captured frames

USAGE:
    screenrecorder stop <output_path>

ARGUMENTS:
    output_path    Directory where captured frames will be saved
                   Must be an existing directory

OPTIONS:
    --verbose, -v  Enable verbose logging

EXAMPLES:
    screenrecorder stop "D:\recordings\session1"
    screenrecorder stop C:\temp\screenshots

NOTES:
    - Frames are saved as JPEG images with timestamps
    - The recording process will terminate after saving
    - Requires an active recording session
)";
}

void CommandLineParser::ShowCancelHelp() {
    std::cout << R"(
CANCEL COMMAND - Cancel recording without saving

USAGE:
    screenrecorder cancel

OPTIONS:
    --verbose, -v  Enable verbose logging

EXAMPLES:
    screenrecorder cancel

NOTES:
    - All captured frames will be discarded
    - The recording process will terminate immediately
    - Requires an active recording session
)";
}

} // namespace ScreenRecorder
