#include "pch.h"
#include "Response.h"

Response::Response()
{
}

Response Response::FromString(const std::string& str)
{
	return Response(DataStream::FromString(str));
}

Response Response::BuildSuccessResponse()
{
	DataStream stream;

	stream.WriteEnum(ResponseType::Success);

	return Response(stream);
}

Response Response::BuildExceptionResponse(const std::exception& e)
{
	DataStream stream;

	stream.WriteEnum(ResponseType::Exception);
	stream.WriteException(e);

	return Response(stream);
}

void Response::ParseExceptionArgs(std::exception& e)
{
	e = m_dataStream.ReadException();
}

ResponseType Response::ParseResponseType()
{
	try
	{
		return m_dataStream.ReadEnum<ResponseType>();
	}
	catch (const std::runtime_error&)
	{
		return ResponseType::Unknown;
	}
}

std::string Response::ToString() const
{
	return m_dataStream.ToString();
}
