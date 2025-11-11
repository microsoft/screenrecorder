#include "pch.h"
#include "ScreenCapture.h"
#include "ScreenRecorderProvider.h"
#include "Logger.h"

namespace ScreenRecorder {

// Bring robmikh.common utilities into scope
using namespace robmikh::common::uwp;
using namespace robmikh::common::desktop;

ScreenCapture::ScreenCapture(
    const winrt::IDirect3DDevice& device,
    const winrt::GraphicsCaptureItem& item,
    int framerate,
    std::shared_ptr<FrameBuffer> frameBuffer)
    : m_device(device)
    , m_item(item)
    , m_frameBuffer(frameBuffer)
    , m_frameInterval(1000 / framerate) {
    
    if (!m_device) {
        throw std::invalid_argument("Direct3D device is null");
    }
    if (!m_item) {
        throw std::invalid_argument("Capture item is null");
    }
    if (!m_frameBuffer) {
        throw std::invalid_argument("Frame buffer is null");
    }
    if (framerate < 1 || framerate > 60) {
        throw std::invalid_argument("Framerate must be between 1 and 60");
    }

    Logger::Instance().Verbose("Initializing screen capture...");
    Logger::Instance().Verbose("Framerate: " + std::to_string(framerate) + " FPS");
    Logger::Instance().Verbose("Frame interval: " + std::to_string(m_frameInterval.count()) + " ms");

    // Get D3D11 device
    m_d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
    m_d3dDevice->GetImmediateContext(m_d3dContext.put());

    // Create frame pool
    auto pixelFormat = winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized;
    m_lastSize = m_item.Size();

    Logger::Instance().Verbose("Creating capture frame pool (size: " +
        std::to_string(m_lastSize.Width) + "x" + std::to_string(m_lastSize.Height) + ")");

    m_framePool = winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(
        m_device,
        pixelFormat,
        2,  // Number of buffers
        m_lastSize);

    // Create capture session
    m_session = m_framePool.CreateCaptureSession(m_item);
    
    // Set up frame arrival handler
    m_frameArrivedRevoker = m_framePool.FrameArrived(
        winrt::auto_revoke,
        { this, &ScreenCapture::OnFrameArrived });

    Logger::Instance().Verbose("Screen capture initialized successfully");
}

ScreenCapture::~ScreenCapture() {
    Stop();
}

void ScreenCapture::Start() {
    CheckClosed();
    
    Logger::Instance().Info("Starting screen capture...");
    
    m_lastFrameTime = std::chrono::steady_clock::now();
    m_framesCaptured = 0;
    
    try {
        m_session.StartCapture();
        Logger::Instance().Info("Screen capture started successfully");
    }
    catch (const winrt::hresult_error& e) {
        std::string errorMsg(winrt::to_string(e.message()));
        Logger::Instance().Error("Failed to start capture: " + errorMsg);
        throw std::runtime_error("Failed to start capture: " + errorMsg);
    }
}

void ScreenCapture::Stop() {
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true)) {
        Logger::Instance().Info("Stopping screen capture...");
        Logger::Instance().Info("Total frames captured: " + std::to_string(m_framesCaptured.load()));

        try {
            // Revoke frame arrival handler
            m_frameArrivedRevoker.revoke();

            // Close session and frame pool
            if (m_session) {
                m_session.Close();
                m_session = nullptr;
            }
            if (m_framePool) {
                m_framePool.Close();
                m_framePool = nullptr;
            }

            m_item = nullptr;

            Logger::Instance().Info("Screen capture stopped successfully");
        }
        catch (const winrt::hresult_error& e) {
            std::string errorMsg(winrt::to_string(e.message()));
            Logger::Instance().Error("Error during capture shutdown: " + errorMsg);
        }
    }
}

void ScreenCapture::SetCursorCaptureEnabled(bool enabled) {
    CheckClosed();
    m_session.IsCursorCaptureEnabled(enabled);
    Logger::Instance().Verbose("Cursor capture " + std::string(enabled ? "enabled" : "disabled"));
}

void ScreenCapture::SetBorderRequired(bool required) {
    CheckClosed();
    m_session.IsBorderRequired(required);
    Logger::Instance().Verbose("Border " + std::string(required ? "required" : "not required"));
}

void ScreenCapture::OnFrameArrived(
    const winrt::Direct3D11CaptureFramePool& sender,
    const winrt::IInspectable&) {
    
    try {
        auto frame = sender.TryGetNextFrame();
        if (!frame) {
            return;
        }

        // Check if enough time has elapsed since last frame
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameTime);

        if (elapsed < m_frameInterval) {
            // Not enough time has passed, skip this frame
            return;
        }

        // Update last frame time
        m_lastFrameTime = now;

        // Generate filename
        std::string filename = GenerateFrameFilename();

        Logger::Instance().Verbose("Capturing frame: " + filename);

        // Get the frame texture
        auto surfaceTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

        // Get texture description
        D3D11_TEXTURE2D_DESC desc{};
        surfaceTexture->GetDesc(&desc);

        // Create a copy of the texture
        winrt::com_ptr<ID3D11Texture2D> frameTexture;
        HRESULT hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, frameTexture.put());
        if (FAILED(hr)) {
            Logger::Instance().Error("Failed to create texture copy");
            return;
        }

        // Copy the texture data
        m_d3dContext->CopyResource(frameTexture.get(), surfaceTexture.get());

        // Add to frame buffer
        m_frameBuffer->AddFrame(frameTexture, filename);
        
        ++m_framesCaptured;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("Error in frame capture: " + std::string(e.what()));
    }
    catch (const winrt::hresult_error& e) {
        std::string errorMsg(winrt::to_string(e.message()));
        Logger::Instance().Error("Error in frame capture: " + errorMsg);
    }
}

void ScreenCapture::CheckClosed() const {
    if (m_closed.load()) {
        throw std::runtime_error("Screen capture is closed");
    }
}

std::string ScreenCapture::GenerateFrameFilename() const {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()) % 1000000;

    std::tm now_tm = {};
    localtime_s(&now_tm, &now_time_t);

    std::ostringstream oss;
    oss << "screenshot_"
        << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S")
        << "-" << std::setfill('0') << std::setw(6) << now_us.count()
        << ".jpg";
    
    return oss.str();
}

// ============================================================================
// ScreenCaptureFactory Implementation
// ============================================================================

std::unique_ptr<ScreenCapture> ScreenCaptureFactory::CreateForMonitor(
    int monitorIndex,
    int framerate,
    std::shared_ptr<FrameBuffer> frameBuffer) {
    
    Logger::Instance().Verbose("Creating screen capture for monitor index: " + std::to_string(monitorIndex));

    // Enumerate monitors
    std::vector<MonitorInfo> monitors = MonitorInfo::EnumerateAll(true);
    
    if (monitors.empty()) {
        throw std::runtime_error("No monitors found");
    }

    // Validate monitor index
    if (monitorIndex < 0 || static_cast<size_t>(monitorIndex) >= monitors.size()) {
        throw std::out_of_range("Monitor index " + std::to_string(monitorIndex) + 
            " is out of range (0-" + std::to_string(monitors.size() - 1) + ")");
    }

    const MonitorInfo& monitor = monitors[monitorIndex];
    Logger::Instance().Info("Capturing monitor: " + 
        std::string(monitor.displayName.begin(), monitor.displayName.end()));

    // Create D3D device
    auto d3dDevice = CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    auto device = CreateDirect3DDevice(dxgiDevice.get());

    // Create capture item
    auto captureItem = CreateCaptureItemForMonitor(monitor.handle);

    // Create screen capture
    return std::make_unique<ScreenCapture>(device, captureItem, framerate, frameBuffer);
}

} // namespace ScreenRecorder
