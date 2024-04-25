#include "pch.h"
#include "DataStream.h"

DataStream::DataStream()
{
}

DataStream::DataStream(const DataStream& other) 
{
    m_stream << other.m_stream.rdbuf();
}

DataStream& DataStream::operator=(const DataStream& other) 
{
    if (this != &other) 
    {
        m_stream.str(other.m_stream.str());
        m_stream.clear(other.m_stream.rdstate());
    }
    return *this;
}

void DataStream::WriteInt(int value) 
{
    m_stream << value << ' ';
}

int DataStream::ReadInt()
{
    int value;
    if (!(m_stream >> value))
        throw std::runtime_error("Error reading int from stream.");
    return value;
}

void DataStream::WriteBool(bool value) 
{
    m_stream << value << ' ';
}

bool DataStream::ReadBool()
{
    bool value;
    if (!(m_stream >> value))
        throw std::runtime_error("Error reading bool from stream.");
    return value;
}

void DataStream::WriteString(const std::string& value) 
{
    m_stream << value.size() << ' ' << value << ' ';
}

std::string DataStream::ReadString()
{
    size_t size;
    if (!(m_stream >> size))
        throw std::runtime_error("Error reading string size from stream.");

    m_stream.ignore(); // Ignore the space character

    std::string value(size, '\0');
    m_stream.read(&value[0], size);
    if (!m_stream)
        throw std::runtime_error("Error reading string data from stream.");

    m_stream.get();
    return value;
}

void DataStream::WriteException(const std::exception& e)
{
    WriteString(e.what());
}

std::exception DataStream::ReadException()
{
    std::string message = ReadString();
    return std::runtime_error(message);
}

std::string DataStream::ToString() const 
{
    return m_stream.str();
}

DataStream DataStream::FromString(const std::string& str) 
{
    DataStream ds;
    ds.m_stream.str(str);
    return ds;
}
