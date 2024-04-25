#include "pch.h"
#include "Client.h"

bool Client::try_connect()
{
    return m_pipe.try_init(L"myPipe", Pipe::CLIENT);
}

bool Client::try_connect(int retries)
{
    while (!try_connect() && retries >= 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        retries--;
    }

    return retries >= 0;
}

Response Client::send(Request& request) const
{
    m_pipe.send(request.ToString());

    return Response::FromString(m_pipe.receive());
}
