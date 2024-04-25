#include "pch.h"
#include "SimpleCapture.h"
#include "ScreenRecorderProvider.h"

namespace winrt
{
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

namespace util
{
    using namespace robmikh::common::uwp;
}

SimpleCapture::SimpleCapture(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device, 
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item, 
    int framerate, CircularFrameBuffer frameBuffer) : m_frameInterval(1000 / framerate), m_frameBuffer(frameBuffer)
{
    m_item = item;
    m_device = device;
    m_fileFormatGuid = winrt::BitmapEncoder::JpegEncoderId();
    m_bitmapPixelFormat = winrt::BitmapPixelFormat::Bgra8;
    auto pixelFormat = winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized;

    m_d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
    m_d3dDevice->GetImmediateContext(m_d3dContext.put());

    // Creating our frame pool with 'Create' instead of 'CreateFreeThreaded'
    // means that the frame pool's FrameArrived event is called on the thread
    // the frame pool was created on. This also means that the creating thread
    // must have a DispatcherQueue. If you use this method, it's best not to do
    // it on the UI thread. 
    m_framePool = winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(m_device, pixelFormat, 2, m_item.Size());
    m_session = m_framePool.CreateCaptureSession(m_item);
    m_lastSize = m_item.Size();
    m_framePool.FrameArrived({ this, &SimpleCapture::OnFrameArrived });
}

void SimpleCapture::StartCapture()
{
    CheckClosed();
    m_lastFrameTime = std::chrono::steady_clock::now();
    m_session.StartCapture();
}

void SimpleCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
        m_session.Close();
        m_framePool.Close();

        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}

void SimpleCapture::CloseAndSave(StorageFolder storageFolder)
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {

        m_session.Close();
        m_framePool.Close();

        m_frameBuffer.save_frames(storageFolder);

        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}

void SimpleCapture::OnFrameArrived(winrt::Direct3D11CaptureFramePool const& sender, winrt::IInspectable const&)
{
    auto frame = sender.TryGetNextFrame();

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastFrame = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameTime).count();
    
    if (timeSinceLastFrame >= m_frameInterval) 
    {
        auto now_sysclock = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now_sysclock);
        auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now_sysclock.time_since_epoch()) % 1000000;
        std::stringstream ss;
        std::tm now_tm = {};
        localtime_s(&now_tm, &now_time_t);
        ss << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S-") << std::setw(6) << std::setfill('0') << now_us.count();
        std::string timestamp = ss.str();
        std::string filename = "screenshot_" + timestamp + ".jpg";

        ReceivedFrameEvent(filename);

        // Store frame

        auto surfaceTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

        D3D11_TEXTURE2D_DESC desc{};
        surfaceTexture->GetDesc(&desc);

        winrt::com_ptr<ID3D11Texture2D> frameTexture;
        winrt::check_hresult(m_d3dDevice->CreateTexture2D(&desc, nullptr, frameTexture.put()));

        m_d3dContext->CopyResource(frameTexture.get(), surfaceTexture.get());

        m_frameBuffer.add_frame(frameTexture, filename);

        m_lastFrameTime = now;
    }
}
