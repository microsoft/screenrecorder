#pragma once

#include "pch.h"

class DataStream {
public:
    DataStream();
    DataStream(const DataStream& other);
    DataStream& operator=(const DataStream& other);

    void WriteInt(int value);
    int ReadInt();

    void WriteBool(bool value);
    bool ReadBool();

    void WriteString(const std::string& value);
    std::string ReadString();

    void WriteException(const std::exception& e);
    std::exception ReadException();

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type
        WriteEnum(T value) {
        m_stream << static_cast<typename std::underlying_type<T>::type>(value) << ' ';
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, T>::type
        ReadEnum() {
        typename std::underlying_type<T>::type intValue;
        if (!(m_stream >> intValue))
            throw std::runtime_error("Error reading enum from stream");
        return static_cast<T>(intValue);
    }

    std::string ToString() const;
    static DataStream FromString(const std::string& str);

private:
    std::stringstream m_stream;
};
