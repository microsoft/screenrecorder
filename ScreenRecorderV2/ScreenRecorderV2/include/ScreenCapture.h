#pragma once
#include "pch.h"
#include "FrameBuffer.h"
#include "MonitorInfo.h"

namespace winrt {
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::DirectX::Direct3D11;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
}

namespace util {
    using namespace robmikh::common::uwp;
    using namespace robmikh::common::desktop;
}

namespace ScreenRecorder {

/// <summary>
/// Screen capture engine using Windows.Graphics.Capture API
/// Captures frames at a specified rate and stores them in a frame buffer
/// </summary>
class ScreenCapture {
public:
    /// <summary>
    /// Create a screen capture instance
    /// </summary>
    /// <param name="device">Direct3D device for capture</param>
    /// <param name="item">Graphics capture item (monitor/window to capture)</param>
    /// <param name="framerate">Capture framerate in FPS</param>
    /// <param name="frameBuffer">Frame buffer for storing captured frames</param>
    ScreenCapture(
        const winrt::IDirect3DDevice& device,
        const winrt::GraphicsCaptureItem& item,
        int framerate,
        std::shared_ptr<FrameBuffer> frameBuffer);

    ~ScreenCapture();

    // Prevent copying
    ScreenCapture(const ScreenCapture&) = delete;
    ScreenCapture& operator=(const ScreenCapture&) = delete;

    /// <summary>
    /// Start capturing frames
    /// </summary>
    void Start();

    /// <summary>
    /// Stop capturing frames
    /// </summary>
    void Stop();

    /// <summary>
    /// Check if capture is currently active
    /// </summary>
    bool IsCapturing() const { return !m_closed.load(); }

    /// <summary>
    /// Get the capture item
    /// </summary>
    winrt::GraphicsCaptureItem GetCaptureItem() const { return m_item; }

    /// <summary>
    /// Enable or disable cursor capture
    /// </summary>
    void SetCursorCaptureEnabled(bool enabled);

    /// <summary>
    /// Enable or disable border
    /// </summary>
    void SetBorderRequired(bool required);

private:
    /// <summary>
    /// Handle frame arrival event
    /// </summary>
    void OnFrameArrived(
        const winrt::Direct3D11CaptureFramePool& sender,
        const winrt::IInspectable& args);

    /// <summary>
    /// Check if capture is closed and throw if it is
    /// </summary>
    void CheckClosed() const;

    /// <summary>
    /// Generate a filename for a captured frame
    /// </summary>
    std::string GenerateFrameFilename() const;

    // Capture objects
    winrt::GraphicsCaptureItem m_item{ nullptr };
    winrt::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::GraphicsCaptureSession m_session{ nullptr };
    winrt::SizeInt32 m_lastSize;
    winrt::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrivedRevoker;

    // Direct3D objects
    winrt::IDirect3DDevice m_device{ nullptr };
    winrt::com_ptr<ID3D11Device> m_d3dDevice{ nullptr };
    winrt::com_ptr<ID3D11DeviceContext> m_d3dContext{ nullptr };

    // Frame buffer
    std::shared_ptr<FrameBuffer> m_frameBuffer;

    // Timing
    std::chrono::steady_clock::time_point m_lastFrameTime;
    std::chrono::milliseconds m_frameInterval;

    // State
    std::atomic<bool> m_closed = false;
    std::atomic<size_t> m_framesCaptured = 0;
};

/// <summary>
/// Factory for creating screen capture instances
/// </summary>
class ScreenCaptureFactory {
public:
    /// <summary>
    /// Create a screen capture for a specific monitor
    /// </summary>
    /// <param name="monitorIndex">Monitor index (0-based, or last index for all monitors)</param>
    /// <param name="framerate">Capture framerate</param>
    /// <param name="frameBuffer">Frame buffer</param>
    /// <returns>Screen capture instance</returns>
    static std::unique_ptr<ScreenCapture> CreateForMonitor(
        int monitorIndex,
        int framerate,
        std::shared_ptr<FrameBuffer> frameBuffer);
};

} // namespace ScreenRecorder
