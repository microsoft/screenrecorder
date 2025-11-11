#include "pch.h"
#include "Logger.h"

namespace ScreenRecorder {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::SetVerbose(bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_verbose = enabled;
}

bool Logger::IsVerbose() const {
    return m_verbose;
}

void Logger::Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Skip verbose messages if verbose mode is not enabled
    if (level == LogLevel::Verbose && !m_verbose) {
        return;
    }

    std::ostream& stream = (level == LogLevel::Error) ? std::cerr : std::cout;

    // Format the log message
    stream << "[" << GetTimestamp() << "] ";

    switch (level) {
        case LogLevel::Error:
            stream << "[ERROR] ";
            break;
        case LogLevel::Info:
            stream << "[INFO] ";
            break;
        case LogLevel::Verbose:
            stream << "[VERBOSE] ";
            break;
    }

    stream << message << std::endl;
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Verbose(const std::string& message) {
    Log(LogLevel::Verbose, message);
}

std::string Logger::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()) % 1000000;

    std::tm now_tm = {};
    localtime_s(&now_tm, &now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H:%M:%S")
        << "." << std::setfill('0') << std::setw(6) << now_us.count();
    return oss.str();
}

} // namespace ScreenRecorder
