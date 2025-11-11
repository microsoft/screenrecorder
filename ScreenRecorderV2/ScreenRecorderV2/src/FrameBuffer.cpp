#include "pch.h"
#include "FrameBuffer.h"
#include "Logger.h"

namespace ScreenRecorder {

// Bring robmikh.common utilities into scope
using namespace robmikh::common::uwp;

FrameBuffer::FrameBuffer(size_t capacity, bool capacityInMegabytes)
    : m_capacity(capacity)
    , m_capacityInMegabytes(capacityInMegabytes)
    , m_currentMemoryUsage(0) {
    
    if (m_capacityInMegabytes) {
        // Convert MB to bytes
        m_capacity *= 1024 * 1024;
        Logger::Instance().Verbose("Frame buffer created with capacity: " + 
            std::to_string(capacity) + " MB");
    }
    else {
        Logger::Instance().Verbose("Frame buffer created with capacity: " + 
            std::to_string(capacity) + " frames");
    }
}

void FrameBuffer::AddFrame(winrt::com_ptr<ID3D11Texture2D> texture, const std::string& filename) {
    if (!texture) {
        Logger::Instance().Error("Attempted to add null texture to frame buffer");
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Calculate frame size
    size_t frameSize = CalculateFrameSize(texture);
    
    // Enforce capacity constraints
    EnforceCapacity(frameSize);

    // Create captured frame
    CapturedFrame frame;
    frame.texture = texture;
    frame.filename = filename;
    frame.sizeBytes = frameSize;
    frame.timestamp = std::chrono::system_clock::now();

    // Add to buffer
    m_frames.push_back(std::move(frame));
    m_currentMemoryUsage += frameSize;

    Logger::Instance().Verbose("Frame added to buffer: " + filename + 
        " (size: " + std::to_string(frameSize / 1024) + " KB, " +
        "total frames: " + std::to_string(m_frames.size()) + ", " +
        "memory: " + std::to_string(m_currentMemoryUsage / (1024 * 1024)) + " MB)");
}

void FrameBuffer::SaveAllFrames(const winrt::Windows::Storage::StorageFolder& storageFolder) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Logger::Instance().Info("Saving " + std::to_string(m_frames.size()) + " frame(s) to disk...");

    size_t savedCount = 0;
    size_t errorCount = 0;

    for (const auto& frame : m_frames) {
        try {
            Logger::Instance().Verbose("Saving frame: " + frame.filename);

            // Create the output file
            auto file = storageFolder.CreateFileAsync(
                winrt::to_hstring(frame.filename),
                winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting).get();

            // Open file stream
            auto stream = file.OpenAsync(winrt::Windows::Storage::FileAccessMode::ReadWrite).get();

            // Create JPEG encoder
            auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(
                winrt::Windows::Graphics::Imaging::BitmapEncoder::JpegEncoderId(),
                stream).get();

            // Get texture description
            D3D11_TEXTURE2D_DESC desc = {};
            frame.texture->GetDesc(&desc);

            // Copy texture bytes
            auto bytes = CopyBytesFromTexture(frame.texture);

            // Set pixel data and encode
            encoder.SetPixelData(
                winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
                winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied,
                desc.Width,
                desc.Height,
                1.0,  // DPI X
                1.0,  // DPI Y
                bytes);

            // Flush to disk
            encoder.FlushAsync().get();
            
            ++savedCount;
            Logger::Instance().Verbose("Frame saved successfully: " + frame.filename);
        }
        catch (const std::exception& e) {
            ++errorCount;
            Logger::Instance().Error("Failed to save frame '" + frame.filename + "': " + e.what());
        }
        catch (const winrt::hresult_error& e) {
            ++errorCount;
            std::string errorMsg(winrt::to_string(e.message()));
            Logger::Instance().Error("Failed to save frame '" + frame.filename + "': " + errorMsg);
        }
    }

    Logger::Instance().Info("Saved " + std::to_string(savedCount) + " frame(s) successfully" +
        (errorCount > 0 ? " (" + std::to_string(errorCount) + " error(s))" : ""));
}

void FrameBuffer::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t frameCount = m_frames.size();
    m_frames.clear();
    m_currentMemoryUsage = 0;
    
    Logger::Instance().Verbose("Frame buffer cleared (" + std::to_string(frameCount) + " frame(s) discarded)");
}

size_t FrameBuffer::GetFrameCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_frames.size();
}

size_t FrameBuffer::GetMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentMemoryUsage;
}

size_t FrameBuffer::CalculateFrameSize(const winrt::com_ptr<ID3D11Texture2D>& texture) const {
    if (!texture) {
        return 0;
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    UINT bitsPerPixel = GetBitsPerPixel(desc.Format);
    size_t size = static_cast<size_t>(desc.Width) * desc.Height * bitsPerPixel / 8;

    return size;
}

void FrameBuffer::EnforceCapacity(size_t incomingFrameSize) {
    // Note: Assumes mutex is already locked by caller

    if (m_capacityInMegabytes) {
        // Remove frames until we have space for the incoming frame
        while (!m_frames.empty() && (m_currentMemoryUsage + incomingFrameSize > m_capacity)) {
            const auto& oldestFrame = m_frames.front();
            m_currentMemoryUsage -= oldestFrame.sizeBytes;
            
            Logger::Instance().Verbose("Removing oldest frame to make space: " + oldestFrame.filename);
            m_frames.pop_front();
        }
    }
    else {
        // Frame count based capacity
        while (m_frames.size() >= m_capacity) {
            const auto& oldestFrame = m_frames.front();
            m_currentMemoryUsage -= oldestFrame.sizeBytes;
            
            Logger::Instance().Verbose("Removing oldest frame (capacity reached): " + oldestFrame.filename);
            m_frames.pop_front();
        }
    }
}

UINT FrameBuffer::GetBitsPerPixel(DXGI_FORMAT format) {
    switch (format) {
        // 128-bit formats
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            return 128;

        // 96-bit formats
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
            return 96;

        // 64-bit formats
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
            return 64;

        // 32-bit formats
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
            return 32;

        // 16-bit formats
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R8G8_TYPELESS:
            return 16;

        // 8-bit formats
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_A8_UNORM:
            return 8;

        default:
            // Default to 32 bits for unknown formats
            Logger::Instance().Verbose("Unknown DXGI format, defaulting to 32 bits per pixel");
            return 32;
    }
}

} // namespace ScreenRecorder
