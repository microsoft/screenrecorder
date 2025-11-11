#pragma once
#include "pch.h"

namespace util {
    using namespace robmikh::common::uwp;
}

namespace ScreenRecorder {

/// <summary>
/// Represents a single captured frame with metadata
/// </summary>
struct CapturedFrame {
    winrt::com_ptr<ID3D11Texture2D> texture;  // Frame texture
    std::string filename;                      // Suggested filename
    size_t sizeBytes;                          // Frame size in bytes
    std::chrono::system_clock::time_point timestamp;  // Capture timestamp
};

/// <summary>
/// Circular buffer for storing captured frames with memory management
/// Thread-safe frame buffer with configurable capacity limits
/// </summary>
class FrameBuffer {
public:
    /// <summary>
    /// Create a frame buffer with specified capacity
    /// </summary>
    /// <param name="capacity">Buffer capacity</param>
    /// <param name="capacityInMegabytes">If true, capacity is in MB; if false, in frame count</param>
    FrameBuffer(size_t capacity, bool capacityInMegabytes);

    ~FrameBuffer() = default;

    // Prevent copying
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    /// <summary>
    /// Add a frame to the buffer (thread-safe)
    /// Oldest frames are automatically discarded if capacity is exceeded
    /// </summary>
    /// <param name="texture">Frame texture</param>
    /// <param name="filename">Suggested filename for the frame</param>
    void AddFrame(winrt::com_ptr<ID3D11Texture2D> texture, const std::string& filename);

    /// <summary>
    /// Save all buffered frames to the specified folder
    /// </summary>
    /// <param name="storageFolder">Target storage folder</param>
    /// <exception cref="std::runtime_error">Thrown if save operation fails</exception>
    void SaveAllFrames(const winrt::Windows::Storage::StorageFolder& storageFolder);

    /// <summary>
    /// Clear all buffered frames
    /// </summary>
    void Clear();

    /// <summary>
    /// Get the number of frames currently in the buffer
    /// </summary>
    size_t GetFrameCount() const;

    /// <summary>
    /// Get the total memory usage of buffered frames in bytes
    /// </summary>
    size_t GetMemoryUsage() const;

private:
    /// <summary>
    /// Calculate the size of a texture in bytes
    /// </summary>
    size_t CalculateFrameSize(const winrt::com_ptr<ID3D11Texture2D>& texture) const;

    /// <summary>
    /// Remove oldest frames until capacity constraints are met
    /// </summary>
    void EnforceCapacity(size_t incomingFrameSize);

    /// <summary>
    /// Get bits per pixel for a DXGI format
    /// </summary>
    static UINT GetBitsPerPixel(DXGI_FORMAT format);

    mutable std::mutex m_mutex;
    std::deque<CapturedFrame> m_frames;
    size_t m_capacity;
    bool m_capacityInMegabytes;
    size_t m_currentMemoryUsage;
};

} // namespace ScreenRecorder
