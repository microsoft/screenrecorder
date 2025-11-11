#pragma once
#include "pch.h"

namespace ScreenRecorder {

/// <summary>
/// Information about a display monitor
/// </summary>
struct MonitorInfo {
    HMONITOR handle;           // Monitor handle
    std::wstring displayName;  // Display name
    RECT bounds;               // Monitor bounds

    /// <summary>
    /// Constructor for a specific monitor
    /// </summary>
    MonitorInfo(HMONITOR hMonitor);

    /// <summary>
    /// Constructor for "all monitors" virtual display
    /// </summary>
    MonitorInfo(HMONITOR hMonitor, const std::wstring& name);

    bool operator==(const MonitorInfo& other) const {
        return handle == other.handle;
    }

    bool operator!=(const MonitorInfo& other) const {
        return !(*this == other);
    }

    /// <summary>
    /// Enumerate all available monitors
    /// </summary>
    /// <param name="includeAllMonitors">If true, includes a virtual "all monitors" option</param>
    /// <returns>Vector of MonitorInfo structures</returns>
    static std::vector<MonitorInfo> EnumerateAll(bool includeAllMonitors = true);
};

} // namespace ScreenRecorder
