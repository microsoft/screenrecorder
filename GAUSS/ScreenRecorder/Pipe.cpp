#include "pch.h"
#include "Pipe.h"

Pipe::Pipe() : m_name(L"mypipe"), m_hpipe(INVALID_HANDLE_VALUE), m_mode(SERVER) 
{
}

bool Pipe::try_init(const std::wstring& name, Mode mode) 
{
    m_name = name;
    m_mode = mode;

    if (m_mode == SERVER) {
        m_hpipe = CreateNamedPipe(
            (L"\\\\.\\pipe\\" + m_name).c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            0,
            0,
            0,
            NULL
        );

        if (m_hpipe == INVALID_HANDLE_VALUE) {
            return false;
        }
    }
    else {
        m_hpipe = CreateFile(
            (L"\\\\.\\pipe\\" + m_name).c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (m_hpipe == INVALID_HANDLE_VALUE) 
        {
            return false;
        }
    }

    return true;
}

Pipe::~Pipe() 
{
    if (m_hpipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hpipe);
    }
}

void Pipe::send(const std::string& message) const
{
    DWORD bytesWritten;

    if (!WriteFile(m_hpipe, message.c_str(), (DWORD)message.size(), &bytesWritten, NULL)) 
    {
        throw std::ios_base::failure("Failed to write to pipe\nWriteFile failed with error " + std::to_string(GetLastError()));
    }
}

std::string Pipe::receive() const
{
    char buffer[1024];
    DWORD bytesRead;

    if (!ReadFile(m_hpipe, buffer, sizeof(buffer), &bytesRead, NULL)) 
    {
        throw std::ios_base::failure("Failed to read from pipe\nReadFile failed with error " + std::to_string(GetLastError()));
    }

    return std::string(buffer, bytesRead);
}

void Pipe::disconnect() const
{
    if (m_mode != SERVER)
    {
        throw std::invalid_argument("Cannot call disconnect on a client side pipe");
    }

    if (!DisconnectNamedPipe(m_hpipe))
    {
        throw std::ios_base::failure("Failed to disconnect from pipe\nDisconnectNamedPipe failed with error " + std::to_string(GetLastError()));
    }
}

void Pipe::connect() const
{
    if (m_mode != SERVER)
    {
        throw std::invalid_argument("Cannot call connect on a client side pipe");
    }

    if (!ConnectNamedPipe(m_hpipe, NULL))
    {
        throw std::ios_base::failure("Failed to disconnect from pipe\nDisconnectNamedPipe failed with error " + std::to_string(GetLastError()));
    }
}