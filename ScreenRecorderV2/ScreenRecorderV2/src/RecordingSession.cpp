#include "pch.h"
#include "RecordingSession.h"
#include "Logger.h"
#include <filesystem>

namespace ScreenRecorder {

// ============================================================================
// RecordingSession Implementation
// ============================================================================

void RecordingSession::Start(const StartParameters& params) {
    if (IsActive()) {
        throw std::runtime_error("Recording session is already active");
    }

    Logger::Instance().Info("Starting recording session...");
    Logger::Instance().Info("  Framerate: " + std::to_string(params.framerate) + " FPS");
    Logger::Instance().Info("  Monitor: " + std::to_string(params.monitorIndex));
    Logger::Instance().Info("  Buffer: " + std::to_string(params.bufferCapacity) + 
        (params.capacityInMegabytes ? " MB" : " frames"));

    try {
        // Create frame buffer
        m_frameBuffer = std::make_shared<FrameBuffer>(
            params.bufferCapacity,
            params.capacityInMegabytes);

        // Create screen capture
        m_capture = ScreenCaptureFactory::CreateForMonitor(
            params.monitorIndex,
            params.framerate,
            m_frameBuffer);

        // Start capture
        m_capture->Start();

        Logger::Instance().Info("Recording session started successfully");
    }
    catch (const std::exception&) {
        m_capture.reset();
        m_frameBuffer.reset();
        throw;
    }
}

void RecordingSession::Stop(const StopParameters& params) {
    if (!IsActive()) {
        throw std::runtime_error("No recording session is active");
    }

    Logger::Instance().Info("Stopping recording session...");
    Logger::Instance().Info("  Output path: " + params.outputPath);

    try {
        // Stop capture
        m_capture->Stop();

        // Get storage folder
        winrt::Windows::Storage::StorageFolder storageFolder = nullptr;
        try {
            storageFolder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(
                winrt::to_hstring(params.outputPath)).get();
        }
        catch (const winrt::hresult_error& e) {
            std::string errorMsg(winrt::to_string(e.message()));
            throw std::runtime_error("Failed to access output folder: " + errorMsg);
        }

        // Save frames
        m_frameBuffer->SaveAllFrames(storageFolder);

        Logger::Instance().Info("Recording session stopped and frames saved successfully");
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Error during stop: " + std::string(e.what()));
        throw;
    }

    // Clean up
    m_capture.reset();
    m_frameBuffer.reset();
}

void RecordingSession::Cancel() {
    if (!IsActive()) {
        throw std::runtime_error("No recording session is active");
    }

    Logger::Instance().Info("Canceling recording session...");

    try {
        // Stop capture
        m_capture->Stop();

        // Clear frame buffer without saving
        m_frameBuffer->Clear();

        Logger::Instance().Info("Recording session canceled");
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Error during cancel: " + std::string(e.what()));
        throw;
    }

    // Clean up
    m_capture.reset();
    m_frameBuffer.reset();
}

// ============================================================================
// RecordingServer Implementation
// ============================================================================

RecordingServer::RecordingServer(const std::wstring& pipeName) {
    Logger::Instance().Verbose("Creating recording server...");
    
    try {
        m_pipeServer = std::make_unique<NamedPipeServer>(pipeName);
        Logger::Instance().Verbose("Recording server created successfully");
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Failed to create recording server: " + std::string(e.what()));
        throw;
    }
}

void RecordingServer::Run() {
    Logger::Instance().Info("Recording server started");

    while (m_running) {
        Logger::Instance().Verbose("Waiting for client connection...");

        // Wait for a client to connect
        if (!m_pipeServer->WaitForClient()) {
            Logger::Instance().Error("Failed to accept client connection");
            continue;
        }

        Logger::Instance().Info("Client connected");

        // Handle client commands
        bool clientConnected = true;
        while (clientConnected && m_running) {
            try {
                // Receive command from client
                IPCMessage command = m_pipeServer->ReceiveMessage();
                
                // Handle the command
                IPCMessage response = HandleCommand(command);

                // Check for shutdown command
                if (command.type == MessageType::Shutdown) {
                    Logger::Instance().Info("Shutdown command received");
                    m_running = false;
                    clientConnected = false;
                }

                // Send response
                m_pipeServer->SendMessage(response);
            }
            catch (const std::exception& e) {
                Logger::Instance().Error("Error handling client: " + std::string(e.what()));
                clientConnected = false;
            }
        }

        // Disconnect client
        m_pipeServer->DisconnectClient();
        Logger::Instance().Verbose("Client disconnected");
    }

    Logger::Instance().Info("Recording server stopped");
}

IPCMessage RecordingServer::HandleCommand(const IPCMessage& command) {
    Logger::Instance().Verbose("Handling command type: " + std::to_string(static_cast<int>(command.type)));

    try {
        switch (command.type) {
            case MessageType::StartRecording: {
                StartParameters params = IPCMessageFactory::ParseStartMessage(command);
                m_session.Start(params);
                return IPCMessageFactory::CreateSuccessMessage("Recording started");
            }

            case MessageType::StopRecording: {
                StopParameters params = IPCMessageFactory::ParseStopMessage(command);
                m_session.Stop(params);
                return IPCMessageFactory::CreateSuccessMessage("Recording stopped and frames saved");
            }

            case MessageType::CancelRecording: {
                m_session.Cancel();
                return IPCMessageFactory::CreateSuccessMessage("Recording canceled");
            }

            case MessageType::Shutdown: {
                return IPCMessageFactory::CreateSuccessMessage("Server shutting down");
            }

            default: {
                std::string error = "Unknown command type: " + std::to_string(static_cast<int>(command.type));
                Logger::Instance().Error(error);
                return IPCMessageFactory::CreateErrorMessage(error);
            }
        }
    }
    catch (const std::exception& e) {
        std::string error = std::string(e.what());
        Logger::Instance().Error("Command failed: " + error);
        return IPCMessageFactory::CreateErrorMessage(error);
    }
}

// ============================================================================
// RecordingClient Implementation
// ============================================================================

RecordingClient::RecordingClient(const std::wstring& pipeName)
    : m_pipeName(pipeName) {
    Logger::Instance().Verbose("Recording client created");
}

bool RecordingClient::IsServerRunning() {
    try {
        NamedPipeClient client(m_pipeName);
        return client.Connect(1000);  // 1 second timeout
    }
    catch (...) {
        return false;
    }
}

void RecordingClient::StartRecording(const StartParameters& params) {
    Logger::Instance().Verbose("Sending start recording command...");
    
    IPCMessage command = IPCMessageFactory::CreateStartMessage(params);
    IPCMessage response = SendCommand(command);

    if (response.type == MessageType::Error) {
        throw std::runtime_error("Start recording failed: " + response.data);
    }

    Logger::Instance().Info("Recording started successfully");
}

void RecordingClient::StopRecording(const StopParameters& params) {
    Logger::Instance().Verbose("Sending stop recording command...");
    
    IPCMessage command = IPCMessageFactory::CreateStopMessage(params);
    IPCMessage response = SendCommand(command);

    if (response.type == MessageType::Error) {
        throw std::runtime_error("Stop recording failed: " + response.data);
    }

    Logger::Instance().Info("Recording stopped successfully");
    
    // Send shutdown command to terminate server
    try {
        IPCMessage shutdownCmd = IPCMessageFactory::CreateShutdownMessage();
        SendCommand(shutdownCmd);
    }
    catch (...) {
        // Ignore errors on shutdown
    }
}

void RecordingClient::CancelRecording() {
    Logger::Instance().Verbose("Sending cancel recording command...");
    
    IPCMessage command = IPCMessageFactory::CreateCancelMessage();
    IPCMessage response = SendCommand(command);

    if (response.type == MessageType::Error) {
        throw std::runtime_error("Cancel recording failed: " + response.data);
    }

    Logger::Instance().Info("Recording canceled successfully");
    
    // Send shutdown command to terminate server
    try {
        IPCMessage shutdownCmd = IPCMessageFactory::CreateShutdownMessage();
        SendCommand(shutdownCmd);
    }
    catch (...) {
        // Ignore errors on shutdown
    }
}

IPCMessage RecordingClient::SendCommand(const IPCMessage& command) {
    NamedPipeClient client(m_pipeName);

    // Connect to server
    if (!client.Connect()) {
        throw std::runtime_error("Failed to connect to recording server");
    }

    // Send command
    client.SendMessage(command);

    // Receive response
    IPCMessage response = client.ReceiveMessage();

    // Disconnect
    client.Disconnect();

    return response;
}

// ============================================================================
// ServerProcessManager Implementation
// ============================================================================

bool ServerProcessManager::StartServerProcess() {
    Logger::Instance().Verbose("Starting recording server process...");

    // Get current executable path
    TCHAR szPath[MAX_PATH];
    if (!GetModuleFileName(nullptr, szPath, MAX_PATH)) {
        Logger::Instance().Error("Failed to get module filename");
        return false;
    }

    // Build command line with -server flag
    std::wstring cmdLine = L"\"" + std::wstring(szPath) + L"\" -server";

    // Setup process creation structures
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = nullptr;
    si.hStdError = nullptr;
    si.hStdInput = nullptr;

    PROCESS_INFORMATION pi = {};

    // Create the server process
    BOOL success = CreateProcessW(
        szPath,                  // Application name
        &cmdLine[0],             // Command line
        nullptr,                 // Process attributes
        nullptr,                 // Thread attributes
        FALSE,                   // Inherit handles
        CREATE_NO_WINDOW,        // Creation flags
        nullptr,                 // Environment
        nullptr,                 // Current directory
        &si,                     // Startup info
        &pi                      // Process information
    );

    if (!success) {
        DWORD error = GetLastError();
        Logger::Instance().Error("Failed to create server process. Error: " + std::to_string(error));
        return false;
    }

    // Close process and thread handles (we don't need them)
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    Logger::Instance().Verbose("Server process started successfully");
    return true;
}

bool ServerProcessManager::WaitForServer(const std::wstring& pipeName, DWORD timeoutMs) {
    Logger::Instance().Verbose("Waiting for server to be ready...");

    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(timeoutMs);

    while (true) {
        // Try to connect
        NamedPipeClient client(pipeName);
        if (client.Connect(1000)) {
            client.Disconnect();
            Logger::Instance().Verbose("Server is ready");
            return true;
        }

        // Check timeout
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (elapsed >= timeout) {
            Logger::Instance().Error("Timeout waiting for server");
            return false;
        }

        // Wait a bit before trying again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace ScreenRecorder
