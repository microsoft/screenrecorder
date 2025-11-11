#pragma once
#include "pch.h"
#include "IPC.h"
#include "ScreenCapture.h"
#include "FrameBuffer.h"

namespace ScreenRecorder {

/// <summary>
/// Manages a recording session including capture and frame buffering
/// </summary>
class RecordingSession {
public:
    RecordingSession() = default;
    ~RecordingSession() = default;

    // Prevent copying
    RecordingSession(const RecordingSession&) = delete;
    RecordingSession& operator=(const RecordingSession&) = delete;

    /// <summary>
    /// Start a new recording session
    /// </summary>
    /// <param name="params">Recording parameters</param>
    /// <exception cref="std::runtime_error">Thrown if recording is already active</exception>
    void Start(const StartParameters& params);

    /// <summary>
    /// Stop recording and save frames to the specified path
    /// </summary>
    /// <param name="params">Stop parameters with output path</param>
    /// <exception cref="std::runtime_error">Thrown if no recording is active</exception>
    void Stop(const StopParameters& params);

    /// <summary>
    /// Cancel recording without saving frames
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if no recording is active</exception>
    void Cancel();

    /// <summary>
    /// Check if a recording session is currently active
    /// </summary>
    bool IsActive() const { return m_capture != nullptr; }

private:
    std::unique_ptr<ScreenCapture> m_capture;
    std::shared_ptr<FrameBuffer> m_frameBuffer;
};

/// <summary>
/// Server that handles recording commands from clients
/// Runs in a separate process and manages the recording session
/// </summary>
class RecordingServer {
public:
    /// <summary>
    /// Create a recording server with the specified pipe name
    /// </summary>
    /// <param name="pipeName">Named pipe for IPC</param>
    explicit RecordingServer(const std::wstring& pipeName);

    ~RecordingServer() = default;

    // Prevent copying
    RecordingServer(const RecordingServer&) = delete;
    RecordingServer& operator=(const RecordingServer&) = delete;

    /// <summary>
    /// Run the server main loop
    /// Handles client connections and processes commands
    /// </summary>
    void Run();

    /// <summary>
    /// Request server shutdown
    /// </summary>
    void Shutdown() { m_running = false; }

private:
    /// <summary>
    /// Handle a single client command
    /// </summary>
    /// <returns>Response message to send to client</returns>
    IPCMessage HandleCommand(const IPCMessage& command);

    std::unique_ptr<NamedPipeServer> m_pipeServer;
    RecordingSession m_session;
    std::atomic<bool> m_running = true;
};

/// <summary>
/// Client for sending commands to the recording server
/// </summary>
class RecordingClient {
public:
    /// <summary>
    /// Create a recording client
    /// </summary>
    /// <param name="pipeName">Named pipe for IPC</param>
    explicit RecordingClient(const std::wstring& pipeName);

    ~RecordingClient() = default;

    // Prevent copying
    RecordingClient(const RecordingClient&) = delete;
    RecordingClient& operator=(const RecordingClient&) = delete;

    /// <summary>
    /// Check if a recording server is running
    /// </summary>
    bool IsServerRunning();

    /// <summary>
    /// Start a new recording session
    /// </summary>
    /// <param name="params">Recording parameters</param>
    /// <exception cref="std::runtime_error">Thrown if command fails</exception>
    void StartRecording(const StartParameters& params);

    /// <summary>
    /// Stop recording and save frames
    /// </summary>
    /// <param name="params">Stop parameters with output path</param>
    /// <exception cref="std::runtime_error">Thrown if command fails</exception>
    void StopRecording(const StopParameters& params);

    /// <summary>
    /// Cancel recording without saving
    /// </summary>
    /// <exception cref="std::runtime_error">Thrown if command fails</exception>
    void CancelRecording();

private:
    /// <summary>
    /// Send a command and wait for response
    /// </summary>
    IPCMessage SendCommand(const IPCMessage& command);

    std::wstring m_pipeName;
};

/// <summary>
/// Utility for managing the recording server process
/// </summary>
class ServerProcessManager {
public:
    /// <summary>
    /// Start a new recording server process
    /// </summary>
    /// <returns>True if server process was started successfully</returns>
    static bool StartServerProcess();

    /// <summary>
    /// Wait for server to be ready
    /// </summary>
    /// <param name="pipeName">Pipe name to check</param>
    /// <param name="timeoutMs">Timeout in milliseconds</param>
    /// <returns>True if server is ready</returns>
    static bool WaitForServer(const std::wstring& pipeName, DWORD timeoutMs = 5000);
};

} // namespace ScreenRecorder
