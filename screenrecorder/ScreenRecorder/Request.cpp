#include "pch.h"
#include "Request.h"

Request::Request() 
{
}

Request Request::FromString(const std::string& str)
{
	return Request(DataStream::FromString(str));
}

Request Request::BuildStartRequest(int framerate, int monitor, int bufferSize, bool isMegabytes)
{
	DataStream stream;

	stream.WriteEnum(RequestType::Start);
	stream.WriteInt(framerate);
	stream.WriteInt(monitor);
	stream.WriteInt(bufferSize);
	stream.WriteBool(isMegabytes);

	return Request(stream);
}

Request Request::BuildStopRequest(const std::string& folder)
{
	DataStream stream;

	stream.WriteEnum(RequestType::Stop);
	stream.WriteString(folder);

	return Request(stream);
}

Request Request::BuildCancelRequest()
{
	DataStream stream;

	stream.WriteEnum(RequestType::Cancel);

	return Request(stream);
}

Request Request::BuildDisconnectRequest()
{
	DataStream stream;

	stream.WriteEnum(RequestType::Disconnect);

	return Request(stream);
}

Request Request::BuildKillRequest()
{
	DataStream stream;

	stream.WriteEnum(RequestType::Kill);

	return Request(stream);
}

void Request::ParseStartArgs(int& framerate, int& monitor, int& bufferSize, bool& isMegabytes) 
{
	framerate = m_dataStream.ReadInt();
	monitor = m_dataStream.ReadInt();
	bufferSize = m_dataStream.ReadInt();
	isMegabytes = m_dataStream.ReadBool();
}

void Request::ParseStopArgs(std::string& folder)
{
	folder = m_dataStream.ReadString();
}

RequestType Request::ParseRequestType()
{
	try
	{
		return m_dataStream.ReadEnum<RequestType>();
	}
	catch (const std::runtime_error&)
	{
		return RequestType::Unknown;
	}
}

std::string Request::ToString() const
{
	return m_dataStream.ToString();
}
