#pragma once

#include "pch.h"

/// <summary>
/// TraceLogging provider for screen recorder events
/// Forward-declare the provider that will be used for logging events
/// </summary>
TRACELOGGING_DECLARE_PROVIDER(g_hScreenRecorderProvider);

/// <summary>
/// Macro for logging frame received events
/// </summary>
/// <param name="filename">Filename of the captured frame</param>
#define ReceivedFrameEvent(filename) \
    TraceLoggingWrite(g_hScreenRecorderProvider, \
        "ReceivedFrame", \
        TraceLoggingString(filename.c_str(), "Filename"))
