#include "pch.h"
#include "IPC.h"
#include "Logger.h"
#include <sstream>

namespace ScreenRecorder {

// Constants
constexpr DWORD PIPE_BUFFER_SIZE = 8192;
constexpr DWORD PIPE_TIMEOUT_MS = 5000;

// ============================================================================
// IPCMessage Implementation
// ============================================================================

std::string IPCMessage::Serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << "|" << data;
    return oss.str();
}

IPCMessage IPCMessage::Deserialize(const std::string& data) {
    size_t delimiter = data.find('|');
    if (delimiter == std::string::npos) {
        throw std::runtime_error("Invalid message format");
    }

    IPCMessage message;
    message.type = static_cast<MessageType>(std::stoi(data.substr(0, delimiter)));
    message.data = data.substr(delimiter + 1);
    return message;
}

// ============================================================================
// NamedPipeServer Implementation
// ============================================================================

NamedPipeServer::NamedPipeServer(const std::wstring& pipeName)
    : m_pipeName(L"\\\\.\\pipe\\" + pipeName) {
    
    Logger::Instance().Verbose("Creating named pipe server: " + 
        std::string(m_pipeName.begin(), m_pipeName.end()));

    HANDLE hPipe = CreateNamedPipeW(
        m_pipeName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,  // Max instances
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        PIPE_TIMEOUT_MS,
        nullptr
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to create named pipe. Error: " + std::to_string(error));
    }

    m_pipeHandle = NamedPipeHandle(hPipe);
    Logger::Instance().Verbose("Named pipe server created successfully");
}

bool NamedPipeServer::WaitForClient() {
    Logger::Instance().Verbose("Waiting for client connection...");

    if (!m_pipeHandle.IsValid()) {
        Logger::Instance().Error("Invalid pipe handle");
        return false;
    }

    BOOL connected = ConnectNamedPipe(m_pipeHandle.Get(), nullptr);
    if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
        DWORD error = GetLastError();
        Logger::Instance().Error("Failed to connect to client. Error: " + std::to_string(error));
        return false;
    }

    m_clientConnected = true;
    Logger::Instance().Verbose("Client connected successfully");
    return true;
}

IPCMessage NamedPipeServer::ReceiveMessage() {
    if (!m_clientConnected) {
        throw std::runtime_error("No client connected");
    }

    Logger::Instance().Verbose("Receiving message from client...");

    char buffer[PIPE_BUFFER_SIZE];
    DWORD bytesRead = 0;

    BOOL success = ReadFile(
        m_pipeHandle.Get(),
        buffer,
        sizeof(buffer) - 1,
        &bytesRead,
        nullptr
    );

    if (!success || bytesRead == 0) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to read from pipe. Error: " + std::to_string(error));
    }

    buffer[bytesRead] = '\0';
    std::string messageData(buffer);
    
    Logger::Instance().Verbose("Received message: " + messageData);
    
    return IPCMessage::Deserialize(messageData);
}

void NamedPipeServer::SendMessage(const IPCMessage& message) {
    if (!m_clientConnected) {
        throw std::runtime_error("No client connected");
    }

    std::string serialized = message.Serialize();
    Logger::Instance().Verbose("Sending message to client: " + serialized);

    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        m_pipeHandle.Get(),
        serialized.c_str(),
        static_cast<DWORD>(serialized.length()),
        &bytesWritten,
        nullptr
    );

    if (!success) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to write to pipe. Error: " + std::to_string(error));
    }

    // Flush to ensure message is sent
    FlushFileBuffers(m_pipeHandle.Get());
    Logger::Instance().Verbose("Message sent successfully");
}

void NamedPipeServer::DisconnectClient() {
    if (m_clientConnected) {
        Logger::Instance().Verbose("Disconnecting client...");
        DisconnectNamedPipe(m_pipeHandle.Get());
        m_clientConnected = false;
        Logger::Instance().Verbose("Client disconnected");
    }
}

// ============================================================================
// NamedPipeClient Implementation
// ============================================================================

NamedPipeClient::NamedPipeClient(const std::wstring& pipeName)
    : m_pipeName(L"\\\\.\\pipe\\" + pipeName) {
    
    Logger::Instance().Verbose("Created named pipe client for: " + 
        std::string(m_pipeName.begin(), m_pipeName.end()));
}

bool NamedPipeClient::Connect(DWORD timeoutMs) {
    Logger::Instance().Verbose("Attempting to connect to server...");

    // Wait for the pipe to become available
    if (!WaitNamedPipeW(m_pipeName.c_str(), timeoutMs)) {
        DWORD error = GetLastError();
        Logger::Instance().Verbose("Pipe not available. Error: " + std::to_string(error));
        return false;
    }

    // Open the pipe
    HANDLE hPipe = CreateFileW(
        m_pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        Logger::Instance().Verbose("Failed to open pipe. Error: " + std::to_string(error));
        return false;
    }

    // Set message mode
    DWORD mode = PIPE_READMODE_MESSAGE;
    BOOL success = SetNamedPipeHandleState(hPipe, &mode, nullptr, nullptr);
    if (!success) {
        DWORD error = GetLastError();
        Logger::Instance().Verbose("Failed to set pipe mode. Error: " + std::to_string(error));
        CloseHandle(hPipe);
        return false;
    }

    m_pipeHandle = NamedPipeHandle(hPipe);
    Logger::Instance().Verbose("Connected to server successfully");
    return true;
}

void NamedPipeClient::SendMessage(const IPCMessage& message) {
    if (!IsConnected()) {
        throw std::runtime_error("Not connected to server");
    }

    std::string serialized = message.Serialize();
    Logger::Instance().Verbose("Sending message to server: " + serialized);

    DWORD bytesWritten = 0;
    BOOL success = WriteFile(
        m_pipeHandle.Get(),
        serialized.c_str(),
        static_cast<DWORD>(serialized.length()),
        &bytesWritten,
        nullptr
    );

    if (!success) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to write to pipe. Error: " + std::to_string(error));
    }

    // Flush to ensure message is sent
    FlushFileBuffers(m_pipeHandle.Get());
    Logger::Instance().Verbose("Message sent successfully");
}

IPCMessage NamedPipeClient::ReceiveMessage() {
    if (!IsConnected()) {
        throw std::runtime_error("Not connected to server");
    }

    Logger::Instance().Verbose("Receiving message from server...");

    char buffer[PIPE_BUFFER_SIZE];
    DWORD bytesRead = 0;

    BOOL success = ReadFile(
        m_pipeHandle.Get(),
        buffer,
        sizeof(buffer) - 1,
        &bytesRead,
        nullptr
    );

    if (!success || bytesRead == 0) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to read from pipe. Error: " + std::to_string(error));
    }

    buffer[bytesRead] = '\0';
    std::string messageData(buffer);
    
    Logger::Instance().Verbose("Received message: " + messageData);
    
    return IPCMessage::Deserialize(messageData);
}

void NamedPipeClient::Disconnect() {
    if (IsConnected()) {
        Logger::Instance().Verbose("Disconnecting from server...");
        m_pipeHandle.Close();
        Logger::Instance().Verbose("Disconnected from server");
    }
}

// ============================================================================
// IPCMessageFactory Implementation
// ============================================================================

IPCMessage IPCMessageFactory::CreateStartMessage(const StartParameters& params) {
    IPCMessage message;
    message.type = MessageType::StartRecording;
    
    std::ostringstream oss;
    oss << params.framerate << ","
        << params.monitorIndex << ","
        << params.bufferCapacity << ","
        << (params.capacityInMegabytes ? "1" : "0");
    message.data = oss.str();
    
    return message;
}

IPCMessage IPCMessageFactory::CreateStopMessage(const StopParameters& params) {
    IPCMessage message;
    message.type = MessageType::StopRecording;
    message.data = params.outputPath;
    return message;
}

IPCMessage IPCMessageFactory::CreateCancelMessage() {
    IPCMessage message;
    message.type = MessageType::CancelRecording;
    message.data = "";
    return message;
}

IPCMessage IPCMessageFactory::CreateShutdownMessage() {
    IPCMessage message;
    message.type = MessageType::Shutdown;
    message.data = "";
    return message;
}

IPCMessage IPCMessageFactory::CreateSuccessMessage(const std::string& details) {
    IPCMessage message;
    message.type = MessageType::Success;
    message.data = details;
    return message;
}

IPCMessage IPCMessageFactory::CreateErrorMessage(const std::string& error) {
    IPCMessage message;
    message.type = MessageType::Error;
    message.data = error;
    return message;
}

StartParameters IPCMessageFactory::ParseStartMessage(const IPCMessage& message) {
    if (message.type != MessageType::StartRecording) {
        throw std::invalid_argument("Message is not a start recording message");
    }

    StartParameters params;
    std::istringstream iss(message.data);
    std::string token;
    
    if (!std::getline(iss, token, ',')) {
        throw std::runtime_error("Invalid start message format");
    }
    params.framerate = std::stoi(token);
    
    if (!std::getline(iss, token, ',')) {
        throw std::runtime_error("Invalid start message format");
    }
    params.monitorIndex = std::stoi(token);
    
    if (!std::getline(iss, token, ',')) {
        throw std::runtime_error("Invalid start message format");
    }
    params.bufferCapacity = std::stoi(token);
    
    if (!std::getline(iss, token, ',')) {
        throw std::runtime_error("Invalid start message format");
    }
    params.capacityInMegabytes = (token == "1");
    
    return params;
}

StopParameters IPCMessageFactory::ParseStopMessage(const IPCMessage& message) {
    if (message.type != MessageType::StopRecording) {
        throw std::invalid_argument("Message is not a stop recording message");
    }

    StopParameters params;
    params.outputPath = message.data;
    return params;
}

} // namespace ScreenRecorder
