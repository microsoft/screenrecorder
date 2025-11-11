#include "pch.h"
#include "CommandLineParser.h"

using namespace ScreenRecorder;

/// <summary>
/// Test fixture for CommandLineParser tests
/// </summary>
class CommandLineParserTest : public ::testing::Test {
protected:
    // Helper to create argv array from string vector
    std::vector<char*> CreateArgv(const std::vector<std::string>& args) {
        argvStorage.clear();
        for (const auto& arg : args) {
            argvStorage.push_back(arg);
        }
        
        std::vector<char*> argv;
        for (auto& arg : argvStorage) {
            argv.push_back(arg.data());
        }
        return argv;
    }

    std::vector<std::string> argvStorage;
};

// ============================================================================
// Basic Command Parsing Tests
// ============================================================================

TEST_F(CommandLineParserTest, NoArguments_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder"});
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartCommand_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Start);
    EXPECT_EQ(result.startParams.framerate, 1);  // Default value
    EXPECT_EQ(result.startParams.monitorIndex, 0);  // Default value
    EXPECT_EQ(result.startParams.bufferCapacity, 100);  // Default value
    EXPECT_TRUE(result.startParams.capacityInMegabytes);  // Default value
    EXPECT_FALSE(result.verbose);
}

TEST_F(CommandLineParserTest, StartCommandLegacyStyle_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "-start"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Start);
}

TEST_F(CommandLineParserTest, StopCommand_ParsedCorrectly) {
    // Create a temporary directory for testing
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "test_output";
    std::filesystem::create_directories(tempDir);
    
    auto argv = CreateArgv({"screenrecorder", "stop", tempDir.string()});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Stop);
    EXPECT_EQ(result.stopParams.outputPath, tempDir.string());
    
    // Cleanup
    std::filesystem::remove(tempDir);
}

TEST_F(CommandLineParserTest, CancelCommand_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "cancel"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Cancel);
}

TEST_F(CommandLineParserTest, HelpCommand_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "help"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Help);
    EXPECT_TRUE(result.helpTopic.empty());
}

TEST_F(CommandLineParserTest, HelpCommandWithTopic_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "help", "start"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Help);
    EXPECT_EQ(result.helpTopic, "start");
}

TEST_F(CommandLineParserTest, UnknownCommand_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "invalid"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, ServerCommand_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "-server"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Server);
}

// ============================================================================
// Start Command Parameter Tests
// ============================================================================

TEST_F(CommandLineParserTest, StartWithFramerate_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "30"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Start);
    EXPECT_EQ(result.startParams.framerate, 30);
}

TEST_F(CommandLineParserTest, StartWithFramerateShortFlag_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "-f", "15"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.framerate, 15);
}

TEST_F(CommandLineParserTest, StartWithMonitor_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--monitor", "1"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.monitorIndex, 1);
}

TEST_F(CommandLineParserTest, StartWithMonitorAll_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--monitor", "-1"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.monitorIndex, -1);
}

TEST_F(CommandLineParserTest, StartWithBufferMB_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "500", "mb"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 500);
    EXPECT_TRUE(result.startParams.capacityInMegabytes);
}

TEST_F(CommandLineParserTest, StartWithBufferMBUppercase_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "500", "MB"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 500);
    EXPECT_TRUE(result.startParams.capacityInMegabytes);
}

TEST_F(CommandLineParserTest, StartWithBufferFrames_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "1000", "frames"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 1000);
    EXPECT_FALSE(result.startParams.capacityInMegabytes);
}

TEST_F(CommandLineParserTest, StartWithBufferNoUnit_DefaultsToMB) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "200"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 200);
    EXPECT_TRUE(result.startParams.capacityInMegabytes);
}

TEST_F(CommandLineParserTest, StartWithMultipleParameters_ParsedCorrectly) {
    auto argv = CreateArgv({
        "screenrecorder", "start", 
        "--framerate", "30", 
        "--monitor", "1", 
        "--buffer", "500", "mb"
    });
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.command, CommandType::Start);
    EXPECT_EQ(result.startParams.framerate, 30);
    EXPECT_EQ(result.startParams.monitorIndex, 1);
    EXPECT_EQ(result.startParams.bufferCapacity, 500);
    EXPECT_TRUE(result.startParams.capacityInMegabytes);
}

// ============================================================================
// Verbose Flag Tests
// ============================================================================

TEST_F(CommandLineParserTest, VerboseFlag_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "--verbose"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_TRUE(result.verbose);
}

TEST_F(CommandLineParserTest, VerboseFlagShort_ParsedCorrectly) {
    auto argv = CreateArgv({"screenrecorder", "start", "-v"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_TRUE(result.verbose);
}

TEST_F(CommandLineParserTest, VerboseFlagInMiddle_ParsedCorrectly) {
    auto argv = CreateArgv({
        "screenrecorder", "start", 
        "--framerate", "30", 
        "--verbose", 
        "--monitor", "1"
    });
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_TRUE(result.verbose);
    EXPECT_EQ(result.startParams.framerate, 30);
    EXPECT_EQ(result.startParams.monitorIndex, 1);
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST_F(CommandLineParserTest, StartWithInvalidFramerateTooLow_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "0"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithInvalidFramerateTooHigh_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "61"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithInvalidMonitorIndex_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--monitor", "-2"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithInvalidBufferCapacityZero_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "0"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithInvalidBufferCapacityTooLargeMB_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "20000", "mb"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithInvalidBufferCapacityTooLargeFrames_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "200000", "frames"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithMissingFramerateValue_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StartWithUnknownParameter_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "start", "--unknown", "value"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

// ============================================================================
// Stop Command Tests
// ============================================================================

TEST_F(CommandLineParserTest, StopWithoutPath_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "stop"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StopWithNonexistentPath_ThrowsException) {
    auto argv = CreateArgv({"screenrecorder", "stop", "C:\\nonexistent\\path"});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
}

TEST_F(CommandLineParserTest, StopWithFilePath_ThrowsException) {
    // Create a temporary file
    std::filesystem::path tempFile = std::filesystem::temp_directory_path() / "test_file.txt";
    std::ofstream(tempFile.string()).close();
    
    auto argv = CreateArgv({"screenrecorder", "stop", tempFile.string()});
    
    EXPECT_THROW(
        CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data()),
        std::invalid_argument
    );
    
    // Cleanup
    std::filesystem::remove(tempFile);
}

// ============================================================================
// Edge Cases and Boundary Tests
// ============================================================================

TEST_F(CommandLineParserTest, BoundaryFramerate_1FPS_Valid) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "1"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.framerate, 1);
}

TEST_F(CommandLineParserTest, BoundaryFramerate_60FPS_Valid) {
    auto argv = CreateArgv({"screenrecorder", "start", "--framerate", "60"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.framerate, 60);
}

TEST_F(CommandLineParserTest, BoundaryBuffer_1MB_Valid) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "1", "mb"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 1);
}

TEST_F(CommandLineParserTest, BoundaryBuffer_10240MB_Valid) {
    auto argv = CreateArgv({"screenrecorder", "start", "--buffer", "10240", "mb"});
    auto result = CommandLineParser::Parse(static_cast<int>(argv.size()), argv.data());
    
    EXPECT_EQ(result.startParams.bufferCapacity, 10240);
}
