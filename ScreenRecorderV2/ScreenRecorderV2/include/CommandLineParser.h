#pragma once
#include "pch.h"

namespace ScreenRecorder {

/// <summary>
/// Command type enumeration
/// </summary>
enum class CommandType {
    Start,      // Start a new recording session
    Stop,       // Stop recording and save frames
    Cancel,     // Cancel recording without saving
    Server,     // Internal: run as server process
    Help,       // Display help information
    Unknown     // Invalid or unrecognized command
};

/// <summary>
/// Parameters for starting a recording
/// </summary>
struct StartParameters {
    int framerate = 1;              // Frames per second (1-60)
    int monitorIndex = 0;           // Monitor index to record (0-based, -1 for all)
    int bufferCapacity = 100;       // Buffer capacity
    bool capacityInMegabytes = true; // True if capacity is in MB, false if in frame count
};

/// <summary>
/// Parameters for stopping a recording
/// </summary>
struct StopParameters {
    std::string outputPath;         // Path where frames should be saved
};

/// <summary>
/// Parsed command-line arguments
/// </summary>
struct ParsedArguments {
    CommandType command = CommandType::Unknown;
    StartParameters startParams;
    StopParameters stopParams;
    bool verbose = false;
    std::string helpTopic;          // Optional topic for help command
};

/// <summary>
/// Command-line argument parser with comprehensive validation and help system
/// </summary>
class CommandLineParser {
public:
    /// <summary>
    /// Parse command-line arguments
    /// </summary>
    /// <param name="argc">Argument count</param>
    /// <param name="argv">Argument values</param>
    /// <returns>Parsed arguments structure</returns>
    /// <exception cref="std::invalid_argument">Thrown for invalid arguments</exception>
    static ParsedArguments Parse(int argc, char* argv[]);

    /// <summary>
    /// Display general help information
    /// </summary>
    static void ShowHelp();

    /// <summary>
    /// Display help for the start command
    /// </summary>
    static void ShowStartHelp();

    /// <summary>
    /// Display help for the stop command
    /// </summary>
    static void ShowStopHelp();

    /// <summary>
    /// Display help for the cancel command
    /// </summary>
    static void ShowCancelHelp();

private:
    /// <summary>
    /// Parse start command arguments
    /// </summary>
    static StartParameters ParseStartArguments(int argc, char* argv[], int startIndex);

    /// <summary>
    /// Parse stop command arguments
    /// </summary>
    static StopParameters ParseStopArguments(int argc, char* argv[], int startIndex);

    /// <summary>
    /// Validate start parameters
    /// </summary>
    static void ValidateStartParameters(const StartParameters& params);
};

} // namespace ScreenRecorder
