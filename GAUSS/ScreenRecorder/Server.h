#pragma once

#include "pch.h"
#include "Pipe.h"
#include "Request.h"
#include "Response.h"
#include "ScreenRecorder.h"

// The purpose of this class is to receive screen recording requests from clients and proces them.
class Server {
public:
    bool try_init();
    void run();

private:
    Response serve_request(Request& request, RequestType requestType);

    Pipe m_pipe;
    ScreenRecorder m_screenRecorder;
    bool m_isRunning;
    bool m_isConnectedToClient;
};