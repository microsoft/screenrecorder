#pragma once

#include "pch.h"

// The purpose of this class is to get the handles to available monitors.
struct MonitorInfo
{
    MonitorInfo(HMONITOR monitorHandle)
    {
        MonitorHandle = monitorHandle;
        MONITORINFOEX monitorInfo = { sizeof(monitorInfo) };
        winrt::check_bool(GetMonitorInfo(MonitorHandle, &monitorInfo));
        std::wstring displayName(monitorInfo.szDevice);
        DisplayName = displayName;
    }
    MonitorInfo(HMONITOR monitorHandle, std::wstring const& displayName)
    {
        MonitorHandle = monitorHandle;
        DisplayName = displayName;
    }

    HMONITOR MonitorHandle;
    std::wstring DisplayName;

    bool operator==(const MonitorInfo& monitor) { return MonitorHandle == monitor.MonitorHandle; }
    bool operator!=(const MonitorInfo& monitor) { return !(*this == monitor); }

    static std::vector<MonitorInfo> EnumerateAllMonitors(bool includeAllMonitors)
    {
        std::vector<MonitorInfo> monitors;
        EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hmon, HDC, LPRECT, LPARAM lparam)
            {
                auto& monitors = *reinterpret_cast<std::vector<MonitorInfo>*>(lparam);
                monitors.push_back(MonitorInfo(hmon));

                return TRUE;
            }, reinterpret_cast<LPARAM>(&monitors));
        if (monitors.size() > 1 && includeAllMonitors)
        {
            monitors.push_back(MonitorInfo(nullptr, L"All Displays"));
        }
        return monitors;
    }
};

