#include "pch.h"
#include "Server.h"

const std::string unknownEnumCaseMessage = "\b\tReceived an unknown request from the main process.\n";
const std::string defaultEnumCaseMessage = "\b\tReceived an unhandled request from the main process.\n";

bool Server::try_init() 
{
    return m_pipe.try_init(L"myPipe", Pipe::SERVER);
}

void Server::run()
{
    m_isRunning = true;

    while (m_isRunning)
    {
        try
        {
            m_pipe.connect();
        }
        catch (const std::ios_base::failure&)
        {
            return;
        }
        
        m_isConnectedToClient = true;

        while (m_isConnectedToClient)
        {
            Request request;
            Response response;

            try
            {
                request = Request::FromString(m_pipe.receive());
            }
            catch (const std::ios_base::failure&)
            {
                m_isConnectedToClient = false;

                break;
            }

            RequestType requestType = request.ParseRequestType();

            if (requestType == RequestType::Disconnect)
            {
                m_isConnectedToClient = false;

                break;
            }

            if (requestType == RequestType::Kill)
            {
                m_isConnectedToClient = false;
                m_isRunning = false;

                break;
            }

            try
            {
                response = serve_request(request, requestType);
            }
            catch (const std::exception& e)
            {
                response = Response::BuildExceptionResponse(e);
            }

            try
            {
                m_pipe.send(response.ToString());
            }
            catch (const std::ios_base::failure&)
            {
                m_isConnectedToClient = false;

                break;
            }
        }

        try
        {
            m_pipe.send(Response::BuildSuccessResponse().ToString());
            m_pipe.disconnect();
        }
        catch (std::ios_base::failure)
        {
            return;
        }  
    }
}

Response Server::serve_request(Request& request, RequestType requestType)
{
    int framerate, monitor, bufferSize;
    bool isMegabytes;
    std::string folder;

    switch (requestType)
    {
    case RequestType::Start:
        request.ParseStartArgs(framerate, monitor, bufferSize, isMegabytes);

        m_screenRecorder.start(framerate, monitor, bufferSize, isMegabytes);

        return Response::BuildSuccessResponse();
    case RequestType::Stop:
        request.ParseStopArgs(folder);

        m_screenRecorder.stop(folder);

        return Response::BuildSuccessResponse();
    case RequestType::Cancel:
        m_screenRecorder.cancel();

        return Response::BuildSuccessResponse();
    case RequestType::Unknown:
        throw std::invalid_argument(unknownEnumCaseMessage);
    default:
        throw std::invalid_argument(defaultEnumCaseMessage);
    }
}