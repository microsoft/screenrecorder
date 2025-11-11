#include "pch.h"
#include "FrameBuffer.h"
#include "TestUtilities.h"

using namespace ScreenRecorder;
using namespace ScreenRecorder::Testing;

/// <summary>
/// Test fixture for FrameBuffer tests
/// </summary>
class FrameBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestUtilities::InitializeD3D();
    }
};

// ============================================================================
// Constructor and Basic Tests
// ============================================================================

TEST_F(FrameBufferTest, Constructor_WithMegabytes_CreatesBuffer) {
    EXPECT_NO_THROW({
        FrameBuffer buffer(100, true);  // 100 MB
    });
}

TEST_F(FrameBufferTest, Constructor_WithFrameCount_CreatesBuffer) {
    EXPECT_NO_THROW({
        FrameBuffer buffer(1000, false);  // 1000 frames
    });
}

TEST_F(FrameBufferTest, NewBuffer_IsEmpty) {
    FrameBuffer buffer(100, true);
    
    EXPECT_EQ(buffer.GetFrameCount(), 0);
    EXPECT_EQ(buffer.GetMemoryUsage(), 0);
}

// ============================================================================
// Add Frame Tests
// ============================================================================

TEST_F(FrameBufferTest, AddFrame_SingleFrame_IncreasesCount) {
    FrameBuffer buffer(100, true);
    auto texture = TestUtilities::CreateMockTexture(1920, 1080);
    
    buffer.AddFrame(texture, "frame_001.jpg");
    
    EXPECT_EQ(buffer.GetFrameCount(), 1);
    EXPECT_GT(buffer.GetMemoryUsage(), 0);
}

TEST_F(FrameBufferTest, AddFrame_MultipleFrames_IncreasesCount) {
    FrameBuffer buffer(100, true);
    
    for (int i = 0; i < 10; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    EXPECT_EQ(buffer.GetFrameCount(), 10);
}

TEST_F(FrameBufferTest, AddFrame_NullTexture_DoesNotCrash) {
    FrameBuffer buffer(100, true);
    
    EXPECT_NO_THROW({
        buffer.AddFrame(nullptr, "frame_001.jpg");
    });
    
    // Should not add null frames
    EXPECT_EQ(buffer.GetFrameCount(), 0);
}

TEST_F(FrameBufferTest, AddFrame_DifferentSizes_TracksMemoryCorrectly) {
    FrameBuffer buffer(100, true);
    
    auto texture1 = TestUtilities::CreateMockTexture(1920, 1080);
    auto texture2 = TestUtilities::CreateMockTexture(1280, 720);
    auto texture3 = TestUtilities::CreateMockTexture(640, 480);
    
    buffer.AddFrame(texture1, "frame_001.jpg");
    size_t memory1 = buffer.GetMemoryUsage();
    
    buffer.AddFrame(texture2, "frame_002.jpg");
    size_t memory2 = buffer.GetMemoryUsage();
    
    buffer.AddFrame(texture3, "frame_003.jpg");
    size_t memory3 = buffer.GetMemoryUsage();
    
    EXPECT_EQ(buffer.GetFrameCount(), 3);
    EXPECT_GT(memory2, memory1);
    EXPECT_GT(memory3, memory2);
}

// ============================================================================
// Capacity Management Tests (Memory-Based)
// ============================================================================

TEST_F(FrameBufferTest, AddFrame_ExceedsMemoryCapacity_RemovesOldestFrames) {
    // Create a buffer that can hold ~2 frames
    FrameBuffer buffer(10, true);  // 10 MB
    
    // Add frames that exceed capacity
    // Each 1920x1080 BGRA frame is ~8 MB
    for (int i = 0; i < 5; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    // Should have removed oldest frames to stay under capacity
    // Should keep only 1 frame (the newest one) since each is ~8MB
    EXPECT_LE(buffer.GetMemoryUsage(), 10 * 1024 * 1024);  // Less than or equal to 10 MB
    EXPECT_GT(buffer.GetFrameCount(), 0);  // But still have some frames
    EXPECT_LE(buffer.GetFrameCount(), 2);  // Should have at most 2 frames
}

TEST_F(FrameBufferTest, AddFrame_MemoryCapacity_KeepsOnlyNewestFrames) {
    FrameBuffer buffer(10, true);  // 10 MB
    
    // Add many small frames
    for (int i = 0; i < 100; ++i) {
        auto texture = TestUtilities::CreateMockTexture(800, 600);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    // Memory should be under or near capacity
    size_t capacityBytes = 10 * 1024 * 1024;
    EXPECT_LE(buffer.GetMemoryUsage(), capacityBytes * 1.1);  // Allow 10% overflow
}

// ============================================================================
// Capacity Management Tests (Frame-Based)
// ============================================================================

TEST_F(FrameBufferTest, AddFrame_ExceedsFrameCapacity_RemovesOldestFrames) {
    FrameBuffer buffer(5, false);  // 5 frames max
    
    // Add 10 frames
    for (int i = 0; i < 10; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    // Should only keep 5 frames (the newest ones)
    EXPECT_EQ(buffer.GetFrameCount(), 5);
}

TEST_F(FrameBufferTest, AddFrame_FrameCapacityBoundary_MaintainsExactCount) {
    FrameBuffer buffer(3, false);  // 3 frames max
    
    // Add exactly 3 frames
    for (int i = 0; i < 3; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    EXPECT_EQ(buffer.GetFrameCount(), 3);
    
    // Add one more (should evict the oldest)
    auto texture = TestUtilities::CreateMockTexture(1920, 1080);
    buffer.AddFrame(texture, "frame_004.jpg");
    
    EXPECT_EQ(buffer.GetFrameCount(), 3);
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(FrameBufferTest, Clear_EmptyBuffer_DoesNotCrash) {
    FrameBuffer buffer(100, true);
    
    EXPECT_NO_THROW({
        buffer.Clear();
    });
    
    EXPECT_EQ(buffer.GetFrameCount(), 0);
    EXPECT_EQ(buffer.GetMemoryUsage(), 0);
}

TEST_F(FrameBufferTest, Clear_BufferWithFrames_RemovesAllFrames) {
    FrameBuffer buffer(100, true);
    
    // Add some frames
    for (int i = 0; i < 10; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    EXPECT_GT(buffer.GetFrameCount(), 0);
    
    buffer.Clear();
    
    EXPECT_EQ(buffer.GetFrameCount(), 0);
    EXPECT_EQ(buffer.GetMemoryUsage(), 0);
}

TEST_F(FrameBufferTest, Clear_ThenAddFrames_WorksCorrectly) {
    FrameBuffer buffer(100, true);
    
    // Add frames
    for (int i = 0; i < 5; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    buffer.Clear();
    
    // Add new frames after clearing
    for (int i = 0; i < 3; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "new_frame_" + std::to_string(i) + ".jpg");
    }
    
    EXPECT_EQ(buffer.GetFrameCount(), 3);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(FrameBufferTest, AddFrame_MultithreadedAccess_ThreadSafe) {
    FrameBuffer buffer(100, true);
    const int numThreads = 4;
    const int framesPerThread = 25;
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&buffer, t, framesPerThread]() {
            for (int i = 0; i < framesPerThread; ++i) {
                auto texture = TestUtilities::CreateMockTexture(800, 600);
                buffer.AddFrame(texture, 
                    "thread_" + std::to_string(t) + "_frame_" + std::to_string(i) + ".jpg");
                
                // Small delay to increase chance of contention
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should have added 100 frames total (or less if capacity was exceeded)
    EXPECT_GT(buffer.GetFrameCount(), 0);
    EXPECT_LE(buffer.GetFrameCount(), numThreads * framesPerThread);
}

TEST_F(FrameBufferTest, GetFrameCount_MultithreadedAccess_ThreadSafe) {
    FrameBuffer buffer(100, true);
    
    // Add some frames
    for (int i = 0; i < 10; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    // Multiple threads reading frame count simultaneously
    std::vector<std::thread> threads;
    std::vector<size_t> counts(10);
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&buffer, &counts, i]() {
            counts[i] = buffer.GetFrameCount();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All counts should be the same
    for (size_t count : counts) {
        EXPECT_EQ(count, 10);
    }
}

// ============================================================================
// Memory Usage Calculation Tests
// ============================================================================

TEST_F(FrameBufferTest, GetMemoryUsage_ReflectsActualTextureSize) {
    FrameBuffer buffer(100, true);
    
    auto texture = TestUtilities::CreateMockTexture(1920, 1080);
    size_t expectedSize = TestUtilities::CalculateTextureSize(1920, 1080, DXGI_FORMAT_B8G8R8A8_UNORM);
    
    buffer.AddFrame(texture, "frame_001.jpg");
    
    size_t actualSize = buffer.GetMemoryUsage();
    
    // Should be approximately the expected size (within 10%)
    EXPECT_GE(actualSize, expectedSize * 0.9);
    EXPECT_LE(actualSize, expectedSize * 1.1);
}

TEST_F(FrameBufferTest, GetMemoryUsage_AccumulatesCorrectly) {
    FrameBuffer buffer(100, true);
    
    // Add first frame
    auto texture1 = TestUtilities::CreateMockTexture(1920, 1080);
    buffer.AddFrame(texture1, "frame_001.jpg");
    size_t memory1 = buffer.GetMemoryUsage();
    
    // Add second frame of same size
    auto texture2 = TestUtilities::CreateMockTexture(1920, 1080);
    buffer.AddFrame(texture2, "frame_002.jpg");
    size_t memory2 = buffer.GetMemoryUsage();
    
    // Memory should have roughly doubled
    EXPECT_GE(memory2, memory1 * 1.8);
    EXPECT_LE(memory2, memory1 * 2.2);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FrameBufferTest, AddFrame_VerySmallCapacity_StillWorks) {
    FrameBuffer buffer(1, true);  // 1 MB - very small
    
    auto texture = TestUtilities::CreateMockTexture(100, 100);  // Small texture
    
    EXPECT_NO_THROW({
        buffer.AddFrame(texture, "frame_001.jpg");
    });
}

TEST_F(FrameBufferTest, AddFrame_SingleFrameCapacity_KeepsOnlyOne) {
    FrameBuffer buffer(1, false);  // 1 frame capacity
    
    for (int i = 0; i < 10; ++i) {
        auto texture = TestUtilities::CreateMockTexture(1920, 1080);
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    EXPECT_EQ(buffer.GetFrameCount(), 1);
}

TEST_F(FrameBufferTest, AddFrame_LargeNumberOfSmallFrames_HandlesCorrectly) {
    FrameBuffer buffer(20, true);  // 20 MB - smaller buffer to force eviction
    
    // Add many small frames
    // Each 100x100 BGRA frame is 40,000 bytes (~40 KB)
    // 1000 frames × 40 KB = ~40 MB total
    const int numFrames = 1000;
    for (int i = 0; i < numFrames; ++i) {
        auto texture = TestUtilities::CreateMockTexture(100, 100);  // ~40 KB each
        buffer.AddFrame(texture, "frame_" + std::to_string(i) + ".jpg");
    }
    
    // Should have kept as many as fit in 20 MB (~500 frames)
    EXPECT_GT(buffer.GetFrameCount(), 0);
    EXPECT_LT(buffer.GetFrameCount(), numFrames);  // Some should have been evicted
    EXPECT_LE(buffer.GetMemoryUsage(), 22 * 1024 * 1024);  // Within 22 MB (allowing small overflow)
}
