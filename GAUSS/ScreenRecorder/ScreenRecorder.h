#pragma once

#include "pch.h"
#include "SimpleCapture.h"

class ScreenRecorder {
public:
    ScreenRecorder();
    ~ScreenRecorder();

    static const int default_framerate = 1;
    static const int default_monitor = 0;
    static const int default_bufferCapacity = 10;
    static const bool default_asMegabytes = false;

    void start(int framerate, int monitor, int bufferCapacity, bool asMegabytes);
    void stop(const std::string& folderPath);
    void cancel();

private:
    std::unique_ptr<SimpleCapture> m_simpleCapture;
    bool isCapturing;
};