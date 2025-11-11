#include "pch.h"
#include "Logger.h"
#include "CommandLineParser.h"
#include "RecordingSession.h"
#include "ScreenRecorderProvider.h"
#include <iostream>

using namespace ScreenRecorder;

// Constants
constexpr const wchar_t* PIPE_NAME = L"ScreenRecorderV2";

/// <summary>
/// Run as server process - handles recording operations
/// </summary>
int RunServer() {
    try {
        Logger::Instance().Info("Screen Recorder Server V2");
        
        // Create and run server
        RecordingServer server(PIPE_NAME);
        server.Run();
        
        return 0;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Server error: " + std::string(e.what()));
        return 1;
    }
}

/// <summary>
/// Handle start recording command
/// </summary>
int HandleStartCommand(const ParsedArguments& args) {
    try {
        RecordingClient client(PIPE_NAME);

        // Check if server is already running
        if (client.IsServerRunning()) {
            Logger::Instance().Error("A recording session is already active");
            return 1;
        }

        // Start server process
        Logger::Instance().Verbose("Starting server process...");
        if (!ServerProcessManager::StartServerProcess()) {
            Logger::Instance().Error("Failed to start recording server");
            return 1;
        }

        // Wait for server to be ready
        if (!ServerProcessManager::WaitForServer(PIPE_NAME, 5000)) {
            Logger::Instance().Error("Server failed to start within timeout period");
            return 1;
        }

        // Send start command
        client.StartRecording(args.startParams);
        
        Logger::Instance().Info("Recording session started successfully");
        Logger::Instance().Info("Use 'screenrecorder stop <path>' to save frames");
        Logger::Instance().Info("Use 'screenrecorder cancel' to discard frames");
        
        return 0;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Failed to start recording: " + std::string(e.what()));
        return 1;
    }
}

/// <summary>
/// Handle stop recording command
/// </summary>
int HandleStopCommand(const ParsedArguments& args) {
    try {
        RecordingClient client(PIPE_NAME);

        // Check if server is running
        if (!client.IsServerRunning()) {
            Logger::Instance().Error("No recording session is active");
            return 1;
        }

        // Send stop command
        client.StopRecording(args.stopParams);
        
        Logger::Instance().Info("Recording saved to: " + args.stopParams.outputPath);
        
        return 0;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Failed to stop recording: " + std::string(e.what()));
        return 1;
    }
}

/// <summary>
/// Handle cancel recording command
/// </summary>
int HandleCancelCommand() {
    try {
        RecordingClient client(PIPE_NAME);

        // Check if server is running
        if (!client.IsServerRunning()) {
            Logger::Instance().Error("No recording session is active");
            return 1;
        }

        // Send cancel command
        client.CancelRecording();
        
        Logger::Instance().Info("Recording canceled");
        
        return 0;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Failed to cancel recording: " + std::string(e.what()));
        return 1;
    }
}

/// <summary>
/// Handle help command
/// </summary>
int HandleHelpCommand(const ParsedArguments& args) {
    if (args.helpTopic.empty()) {
        CommandLineParser::ShowHelp();
    }
    else if (args.helpTopic == "start") {
        CommandLineParser::ShowStartHelp();
    }
    else if (args.helpTopic == "stop") {
        CommandLineParser::ShowStopHelp();
    }
    else if (args.helpTopic == "cancel") {
        CommandLineParser::ShowCancelHelp();
    }
    else {
        Logger::Instance().Error("Unknown help topic: " + args.helpTopic);
        CommandLineParser::ShowHelp();
        return 1;
    }
    
    return 0;
}

/// <summary>
/// Main entry point
/// </summary>
int main(int argc, char* argv[]) {
    // Initialize COM for WinRT
    winrt::init_apartment();

    // Register TraceLogging provider
    TraceLoggingRegister(g_hScreenRecorderProvider);

    try {
        // Parse command-line arguments
        ParsedArguments args;
        
        try {
            args = CommandLineParser::Parse(argc, argv);
        }
        catch (const std::invalid_argument& e) {
            Logger::Instance().Error(e.what());
            std::cout << "\n";
            CommandLineParser::ShowHelp();
            return 1;
        }

        // Set verbose mode if requested
        if (args.verbose) {
            Logger::Instance().SetVerbose(true);
            Logger::Instance().Verbose("Verbose logging enabled");
        }

        // Route to appropriate handler
        switch (args.command) {
            case CommandType::Server:
                return RunServer();

            case CommandType::Start:
                return HandleStartCommand(args);

            case CommandType::Stop:
                return HandleStopCommand(args);

            case CommandType::Cancel:
                return HandleCancelCommand();

            case CommandType::Help:
                return HandleHelpCommand(args);

            case CommandType::Unknown:
            default:
                Logger::Instance().Error("Unknown command");
                CommandLineParser::ShowHelp();
                return 1;
        }
    }
    catch (const winrt::hresult_error& e) {
        std::string errorMsg(winrt::to_string(e.message()));
        Logger::Instance().Error("WinRT error: " + errorMsg);
        return 1;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Unexpected error: " + std::string(e.what()));
        return 1;
    }
    catch (...) {
        Logger::Instance().Error("Unknown error occurred");
        return 1;
    }

    // Unregister TraceLogging provider
    TraceLoggingUnregister(g_hScreenRecorderProvider);

    return 0;
}
