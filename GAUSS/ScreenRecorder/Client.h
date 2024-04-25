#pragma once

#include "pch.h"
#include "Pipe.h"
#include "Request.h"
#include "Response.h"

class Client {
public:
    bool try_connect();
    bool try_connect(int retries);

    Response send(Request& request) const;

private:
    Pipe m_pipe;
};
