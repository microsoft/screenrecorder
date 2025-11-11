#include "pch.h"
#include "Logger.h"

using namespace ScreenRecorder;

/// <summary>
/// Test fixture for Logger tests
/// </summary>
class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset logger to default state before each test
        Logger::Instance().SetVerbose(false);
    }

    void TearDown() override {
        // Reset logger after each test
        Logger::Instance().SetVerbose(false);
    }
};

// ============================================================================
// Singleton Pattern Tests
// ============================================================================

TEST_F(LoggerTest, Instance_ReturnsValidLogger) {
    Logger& logger = Logger::Instance();
    EXPECT_NE(&logger, nullptr);
}

TEST_F(LoggerTest, Instance_ReturnsSameInstance) {
    Logger& logger1 = Logger::Instance();
    Logger& logger2 = Logger::Instance();
    
    EXPECT_EQ(&logger1, &logger2);
}

// ============================================================================
// Verbose Mode Tests
// ============================================================================

TEST_F(LoggerTest, SetVerbose_EnablesVerboseMode) {
    Logger::Instance().SetVerbose(true);
    EXPECT_TRUE(Logger::Instance().IsVerbose());
}

TEST_F(LoggerTest, SetVerbose_DisablesVerboseMode) {
    Logger::Instance().SetVerbose(true);
    Logger::Instance().SetVerbose(false);
    EXPECT_FALSE(Logger::Instance().IsVerbose());
}

TEST_F(LoggerTest, IsVerbose_DefaultIsFalse) {
    EXPECT_FALSE(Logger::Instance().IsVerbose());
}

// ============================================================================
// Logging Level Tests
// ============================================================================

TEST_F(LoggerTest, Error_AlwaysLogs) {
    testing::internal::CaptureStderr();
    
    Logger::Instance().Error("Test error message");
    
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Test error message"), std::string::npos);
}

TEST_F(LoggerTest, Info_LogsInNormalMode) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info("Test info message");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test info message"), std::string::npos);
}

TEST_F(LoggerTest, Info_LogsInVerboseMode) {
    Logger::Instance().SetVerbose(true);
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info("Test info message");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test info message"), std::string::npos);
}

TEST_F(LoggerTest, Verbose_DoesNotLogInNormalMode) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Verbose("Test verbose message");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output.find("Test verbose message"), std::string::npos);
}

TEST_F(LoggerTest, Verbose_LogsInVerboseMode) {
    Logger::Instance().SetVerbose(true);
    testing::internal::CaptureStdout();
    
    Logger::Instance().Verbose("Test verbose message");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test verbose message"), std::string::npos);
}

// ============================================================================
// Generic Log Method Tests
// ============================================================================

TEST_F(LoggerTest, Log_ErrorLevel_AlwaysLogs) {
    testing::internal::CaptureStderr();
    
    Logger::Instance().Log(LogLevel::Error, "Test error via Log");
    
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Test error via Log"), std::string::npos);
}

TEST_F(LoggerTest, Log_InfoLevel_LogsInNormalMode) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Log(LogLevel::Info, "Test info via Log");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test info via Log"), std::string::npos);
}

TEST_F(LoggerTest, Log_VerboseLevel_DoesNotLogInNormalMode) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Log(LogLevel::Verbose, "Test verbose via Log");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output.find("Test verbose via Log"), std::string::npos);
}

TEST_F(LoggerTest, Log_VerboseLevel_LogsInVerboseMode) {
    Logger::Instance().SetVerbose(true);
    testing::internal::CaptureStdout();
    
    Logger::Instance().Log(LogLevel::Verbose, "Test verbose via Log");
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Test verbose via Log"), std::string::npos);
}

// ============================================================================
// Message Content Tests
// ============================================================================

TEST_F(LoggerTest, Log_IncludesTimestamp) {
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info("Test message");
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Check for timestamp pattern (e.g., [HH:MM:SS])
    EXPECT_NE(output.find("["), std::string::npos);
    EXPECT_NE(output.find("]"), std::string::npos);
    EXPECT_NE(output.find(":"), std::string::npos);
}

TEST_F(LoggerTest, Log_EmptyMessage_DoesNotCrash) {
    EXPECT_NO_THROW({
        testing::internal::CaptureStdout();
        Logger::Instance().Info("");
        testing::internal::GetCapturedStdout();
    });
}

TEST_F(LoggerTest, Log_LongMessage_LogsCompletely) {
    std::string longMessage(1000, 'A');
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info(longMessage);
    
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find(longMessage), std::string::npos);
}

TEST_F(LoggerTest, Log_SpecialCharacters_LogsCorrectly) {
    std::string specialMessage = "Test\nNew\tLine\r\nSpecial!@#$%^&*()";
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info(specialMessage);
    
    std::string output = testing::internal::GetCapturedStdout();
    // Should contain at least part of the message
    EXPECT_NE(output.find("Test"), std::string::npos);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(LoggerTest, Log_MultithreadedAccess_ThreadSafe) {
    const int numThreads = 10;
    const int messagesPerThread = 100;
    
    std::vector<std::thread> threads;
    
    testing::internal::CaptureStdout();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t, messagesPerThread]() {
            for (int i = 0; i < messagesPerThread; ++i) {
                Logger::Instance().Info(
                    "Thread " + std::to_string(t) + 
                    " Message " + std::to_string(i)
                );
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Should have logged all messages (check for a few)
    EXPECT_NE(output.find("Thread 0"), std::string::npos);
    EXPECT_NE(output.find("Thread 5"), std::string::npos);
    EXPECT_NE(output.find("Thread 9"), std::string::npos);
}

TEST_F(LoggerTest, SetVerbose_MultithreadedAccess_ThreadSafe) {
    const int numThreads = 10;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < 100; ++i) {
                Logger::Instance().SetVerbose(t % 2 == 0);
                bool isVerbose = Logger::Instance().IsVerbose();
                // Just ensure no crashes - result may vary due to race conditions
                (void)isVerbose;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Test should complete without crashes
    SUCCEED();
}

TEST_F(LoggerTest, Log_MixedLevelsMultithreaded_ThreadSafe) {
    const int numThreads = 5;
    
    Logger::Instance().SetVerbose(true);
    
    std::vector<std::thread> threads;
    
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([t]() {
            for (int i = 0; i < 50; ++i) {
                switch (i % 3) {
                    case 0:
                        Logger::Instance().Error("Error from thread " + std::to_string(t));
                        break;
                    case 1:
                        Logger::Instance().Info("Info from thread " + std::to_string(t));
                        break;
                    case 2:
                        Logger::Instance().Verbose("Verbose from thread " + std::to_string(t));
                        break;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    testing::internal::GetCapturedStdout();
    testing::internal::GetCapturedStderr();
    
    // Test should complete without crashes or deadlocks
    SUCCEED();
}

// ============================================================================
// State Persistence Tests
// ============================================================================

TEST_F(LoggerTest, VerboseMode_PersistsAcrossLogs) {
    Logger::Instance().SetVerbose(true);
    
    // Log several messages
    testing::internal::CaptureStdout();
    Logger::Instance().Verbose("Message 1");
    Logger::Instance().Info("Message 2");
    Logger::Instance().Verbose("Message 3");
    std::string output = testing::internal::GetCapturedStdout();
    
    // All verbose messages should appear
    EXPECT_NE(output.find("Message 1"), std::string::npos);
    EXPECT_NE(output.find("Message 2"), std::string::npos);
    EXPECT_NE(output.find("Message 3"), std::string::npos);
    
    // Verbose mode should still be enabled
    EXPECT_TRUE(Logger::Instance().IsVerbose());
}

TEST_F(LoggerTest, VerboseMode_CanBeToggledMultipleTimes) {
    for (int i = 0; i < 10; ++i) {
        Logger::Instance().SetVerbose(i % 2 == 0);
        EXPECT_EQ(Logger::Instance().IsVerbose(), i % 2 == 0);
    }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(LoggerTest, RealWorldScenario_StartupLogging) {
    Logger::Instance().SetVerbose(true);
    
    testing::internal::CaptureStdout();
    
    Logger::Instance().Info("Screen Recorder V2 starting...");
    Logger::Instance().Verbose("Initializing D3D11 device");
    Logger::Instance().Verbose("Creating frame buffer with 100 MB capacity");
    Logger::Instance().Info("Recording session started");
    
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_NE(output.find("Screen Recorder V2 starting"), std::string::npos);
    EXPECT_NE(output.find("Initializing D3D11"), std::string::npos);
    EXPECT_NE(output.find("Creating frame buffer"), std::string::npos);
    EXPECT_NE(output.find("Recording session started"), std::string::npos);
}

TEST_F(LoggerTest, RealWorldScenario_ErrorHandling) {
    testing::internal::CaptureStderr();
    
    Logger::Instance().Error("Failed to create D3D11 device");
    Logger::Instance().Error("Recording session terminated");
    
    std::string output = testing::internal::GetCapturedStderr();
    
    EXPECT_NE(output.find("Failed to create D3D11 device"), std::string::npos);
    EXPECT_EQ(output.find("Test message"), std::string::npos);
}
