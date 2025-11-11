#include "pch.h"
#include "ScreenRecorderProvider.h"

/// <summary>
/// Define the TraceLogging provider for screen recorder
/// GUID: {fe8fc3d0-1e6a-42f2-be28-9f8a0fcf7b04}
/// </summary>
TRACELOGGING_DEFINE_PROVIDER(
    g_hScreenRecorderProvider,
    "ScreenRecorderV2",
    (0xfe8fc3d0, 0x1e6a, 0x42f2, 0xbe, 0x28, 0x9f, 0x8a, 0x0f, 0xcf, 0x7b, 0x04));
