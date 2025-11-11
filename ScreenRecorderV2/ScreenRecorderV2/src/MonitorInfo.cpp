#include "pch.h"
#include "MonitorInfo.h"
#include "Logger.h"

namespace ScreenRecorder {

MonitorInfo::MonitorInfo(HMONITOR hMonitor) : handle(hMonitor), bounds{} {
    if (hMonitor) {
        MONITORINFOEXW monitorInfo = { sizeof(monitorInfo) };
        if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
            displayName = monitorInfo.szDevice;
            bounds = monitorInfo.rcMonitor;
            
            Logger::Instance().Verbose("Monitor detected: " + 
                std::string(displayName.begin(), displayName.end()));
        }
        else {
            throw std::runtime_error("Failed to get monitor information");
        }
    }
}

MonitorInfo::MonitorInfo(HMONITOR hMonitor, const std::wstring& name)
    : handle(hMonitor), displayName(name), bounds{} {
}

std::vector<MonitorInfo> MonitorInfo::EnumerateAll(bool includeAllMonitors) {
    Logger::Instance().Verbose("Enumerating monitors...");
    
    std::vector<MonitorInfo> monitors;
    
    // Enumerate all monitors
    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto& monitors = *reinterpret_cast<std::vector<MonitorInfo>*>(lParam);
            try {
                monitors.push_back(MonitorInfo(hMonitor));
            }
            catch (const std::exception& e) {
                Logger::Instance().Error("Failed to add monitor: " + std::string(e.what()));
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&monitors));

    Logger::Instance().Verbose("Found " + std::to_string(monitors.size()) + " monitor(s)");

    // Add "all monitors" option if requested and multiple monitors exist
    if (includeAllMonitors && monitors.size() > 1) {
        monitors.push_back(MonitorInfo(nullptr, L"All Displays"));
        Logger::Instance().Verbose("Added 'All Displays' option");
    }

    return monitors;
}

} // namespace ScreenRecorder
