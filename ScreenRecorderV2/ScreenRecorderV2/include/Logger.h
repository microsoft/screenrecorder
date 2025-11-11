#pragma once
#include "pch.h"

namespace ScreenRecorder {

/// <summary>
/// Logging level enumeration
/// </summary>
enum class LogLevel {
    Error,   // Always shown - critical errors
    Info,    // Normal output - general information
    Verbose  // Verbose mode only - detailed debugging information
};

/// <summary>
/// Thread-safe logger with support for verbose mode
/// Provides timestamped, formatted logging output to stdout/stderr
/// </summary>
class Logger {
public:
    /// <summary>
    /// Get the singleton logger instance
    /// </summary>
    static Logger& Instance();

    /// <summary>
    /// Set verbose logging mode
    /// </summary>
    /// <param name="enabled">True to enable verbose logging</param>
    void SetVerbose(bool enabled);

    /// <summary>
    /// Check if verbose mode is enabled
    /// </summary>
    bool IsVerbose() const;

    /// <summary>
    /// Log a message at the specified level
    /// </summary>
    void Log(LogLevel level, const std::string& message);

    /// <summary>
    /// Log an error message (always shown)
    /// </summary>
    void Error(const std::string& message);

    /// <summary>
    /// Log an info message (shown in normal and verbose modes)
    /// </summary>
    void Info(const std::string& message);

    /// <summary>
    /// Log a verbose message (shown only in verbose mode)
    /// </summary>
    void Verbose(const std::string& message);

private:
    Logger() = default;
    ~Logger() = default;

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /// <summary>
    /// Format timestamp for log messages
    /// </summary>
    std::string GetTimestamp() const;

    std::mutex m_mutex;
    bool m_verbose = false;
};

} // namespace ScreenRecorder
