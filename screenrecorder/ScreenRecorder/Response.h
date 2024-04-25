#pragma once

#include "pch.h"
#include "DataStream.h"

enum class ResponseType { Success, Exception, Unknown };

class Response {
public:
    Response();

    static Response FromString(const std::string& str);

    static Response BuildSuccessResponse();
    static Response BuildExceptionResponse(const std::exception& e);

    void ParseExceptionArgs(std::exception& e);

    ResponseType ParseResponseType();

    std::string ToString() const;

private:
    Response(const DataStream& dataStream) : m_dataStream(dataStream) {}

    DataStream m_dataStream;
};
