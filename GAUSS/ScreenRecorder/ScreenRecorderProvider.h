#pragma once

#include "pch.h"

// Forward-declare the g_hMyComponentProvider variable that will be used in any class that wants to log events for the screen recorder.
TRACELOGGING_DECLARE_PROVIDER(g_hMyComponentProvider);

#define ReceivedFrameEvent(filename) \
    TraceLoggingWrite(g_hMyComponentProvider, \
        "ReceivedFrame", \
        TraceLoggingString(filename.c_str(), "Filename"))


