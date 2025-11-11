#pragma once
#include "pch.h"
#include "CommandLineParser.h"

namespace ScreenRecorder {

/// <summary>
/// IPC message types for client-server communication
/// </summary>
enum class MessageType {
    StartRecording,     // Client -> Server: Start recording with parameters
    StopRecording,      // Client -> Server: Stop and save recording
    CancelRecording,    // Client -> Server: Cancel recording without saving
    Shutdown,           // Client -> Server: Shutdown server process
    Success,            // Server -> Client: Operation completed successfully
    Error               // Server -> Client: Operation failed with error message
};

/// <summary>
/// IPC message structure for communication between client and server
/// </summary>
struct IPCMessage {
    MessageType type;
    std::string data;  // JSON-serialized message data

    /// <summary>
    /// Serialize message to string for transmission
    /// </summary>
    std::string Serialize() const;

    /// <summary>
    /// Deserialize message from string
    /// </summary>
    static IPCMessage Deserialize(const std::string& data);
};

/// <summary>
/// RAII wrapper for Windows named pipe handle
/// </summary>
class NamedPipeHandle {
public:
    NamedPipeHandle() : m_handle(INVALID_HANDLE_VALUE) {}
    
    explicit NamedPipeHandle(HANDLE handle) : m_handle(handle) {}
    
    ~NamedPipeHandle() {
        Close();
    }

    // Prevent copying
    NamedPipeHandle(const NamedPipeHandle&) = delete;
    NamedPipeHandle& operator=(const NamedPipeHandle&) = delete;

    // Allow moving
    NamedPipeHandle(NamedPipeHandle&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = INVALID_HANDLE_VALUE;
    }

    NamedPipeHandle& operator=(NamedPipeHandle&& other) noexcept {
        if (this != &other) {
            Close();
            m_handle = other.m_handle;
            other.m_handle = INVALID_HANDLE_VALUE;
        }
        return *this;
    }

    /// <summary>
    /// Get the underlying handle
    /// </summary>
    HANDLE Get() const { return m_handle; }

    /// <summary>
    /// Check if handle is valid
    /// </summary>
    bool IsValid() const { return m_handle != INVALID_HANDLE_VALUE; }

    /// <summary>
    /// Release ownership of the handle
    /// </summary>
    HANDLE Release() {
        HANDLE h = m_handle;
        m_handle = INVALID_HANDLE_VALUE;
        return h;
    }

    /// <summary>
    /// Close the handle
    /// </summary>
    void Close() {
        if (IsValid()) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
    }

private:
    HANDLE m_handle;
};

/// <summary>
/// Named pipe server for receiving commands from clients
/// </summary>
class NamedPipeServer {
public:
    /// <summary>
    /// Create a named pipe server
    /// </summary>
    /// <param name="pipeName">Name of the pipe (without \\.\pipe\ prefix)</param>
    explicit NamedPipeServer(const std::wstring& pipeName);

    ~NamedPipeServer() = default;

    // Prevent copying
    NamedPipeServer(const NamedPipeServer&) = delete;
    NamedPipeServer& operator=(const NamedPipeServer&) = delete;

    /// <summary>
    /// Wait for a client to connect
    /// </summary>
    /// <returns>True if client connected successfully</returns>
    bool WaitForClient();

    /// <summary>
    /// Receive a message from the connected client
    /// </summary>
    /// <returns>Received message</returns>
    /// <exception cref="std::runtime_error">Thrown if receive fails</exception>
    IPCMessage ReceiveMessage();

    /// <summary>
    /// Send a message to the connected client
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if send fails</exception>
    void SendMessage(const IPCMessage& message);

    /// <summary>
    /// Disconnect the current client
    /// </summary>
    void DisconnectClient();

private:
    std::wstring m_pipeName;
    NamedPipeHandle m_pipeHandle;
    bool m_clientConnected = false;
};

/// <summary>
/// Named pipe client for sending commands to server
/// </summary>
class NamedPipeClient {
public:
    /// <summary>
    /// Create a named pipe client
    /// </summary>
    /// <param name="pipeName">Name of the pipe (without \\.\pipe\ prefix)</param>
    explicit NamedPipeClient(const std::wstring& pipeName);

    ~NamedPipeClient() = default;

    // Prevent copying
    NamedPipeClient(const NamedPipeClient&) = delete;
    NamedPipeClient& operator=(const NamedPipeClient&) = delete;

    /// <summary>
    /// Connect to the server
    /// </summary>
    /// <param name="timeoutMs">Timeout in milliseconds (0 = wait indefinitely)</param>
    /// <returns>True if connected successfully</returns>
    bool Connect(DWORD timeoutMs = 5000);

    /// <summary>
    /// Check if currently connected to server
    /// </summary>
    bool IsConnected() const { return m_pipeHandle.IsValid(); }

    /// <summary>
    /// Send a message to the server
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if send fails</exception>
    void SendMessage(const IPCMessage& message);

    /// <summary>
    /// Receive a message from the server
    /// </summary>
    /// <returns>Received message</returns>
    /// <exception cref="std::runtime_error">Thrown if receive fails</exception>
    IPCMessage ReceiveMessage();

    /// <summary>
    /// Disconnect from the server
    /// </summary>
    void Disconnect();

private:
    std::wstring m_pipeName;
    NamedPipeHandle m_pipeHandle;
};

/// <summary>
/// Helper functions for creating IPC messages
/// </summary>
class IPCMessageFactory {
public:
    static IPCMessage CreateStartMessage(const StartParameters& params);
    static IPCMessage CreateStopMessage(const StopParameters& params);
    static IPCMessage CreateCancelMessage();
    static IPCMessage CreateShutdownMessage();
    static IPCMessage CreateSuccessMessage(const std::string& details = "");
    static IPCMessage CreateErrorMessage(const std::string& error);

    static StartParameters ParseStartMessage(const IPCMessage& message);
    static StopParameters ParseStopMessage(const IPCMessage& message);
};

} // namespace ScreenRecorder
