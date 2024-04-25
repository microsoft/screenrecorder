#pragma once

#include "pch.h"
#include "DataStream.h"

enum class RequestType { Start, Stop, Cancel, Disconnect, Kill, Unknown };

class Request {
public:
    Request();

    static Request FromString(const std::string& str);

    static Request BuildStartRequest(int arg1, int arg2, int arg3, bool arg4);
    static Request BuildStopRequest(const std::string& arg1);
    static Request BuildCancelRequest();
    static Request BuildDisconnectRequest();
    static Request BuildKillRequest();

    void ParseStartArgs(int& arg1, int& arg2, int& arg3, bool& arg4);
    void ParseStopArgs(std::string& arg1);

    RequestType ParseRequestType();

    std::string ToString() const;

private:
    Request(const DataStream& dataStream) : m_dataStream(dataStream) {}

    DataStream m_dataStream;
};
 