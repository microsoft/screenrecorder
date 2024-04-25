#include "pch.h"
#include "CircularFrameBuffer.h"

CircularFrameBuffer::CircularFrameBuffer(size_t capacity, bool asMegabytes) : m_capacity(capacity), m_asMegabytes(asMegabytes), m_memoryUsage(0)
{
    if (asMegabytes) 
    {
        m_capacity *= 1000000;
    }
}

void CircularFrameBuffer::add_frame(winrt::com_ptr<ID3D11Texture2D> texture, const std::string& filename) 
{
    size_t frame_size = calculate_frame_size(texture);

    if (m_asMegabytes) 
    {
        while (m_memoryUsage + frame_size > m_capacity && !m_frames.empty()) 
        {
            m_memoryUsage -= m_frames.front().size;
            m_frames.pop_front();
        }
    }
    else if (m_frames.size() == m_capacity)
    {
        m_memoryUsage -= m_frames.front().size;
        m_frames.pop_front();
    }

    m_frames.push_back({ texture, filename, frame_size });
    m_memoryUsage += frame_size;
}

size_t CircularFrameBuffer::calculate_frame_size(winrt::com_ptr<ID3D11Texture2D> texture) 
{
    if (!texture)
        return 0;

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    UINT bpp = 0;
    switch (desc.Format)
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        bpp = 128;
        break;
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        bpp = 96;
        break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
        bpp = 64;
        break;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
        bpp = 32;
        break;
    }

    size_t size = desc.Width * desc.Height * bpp / 8;

    return size;
}

void CircularFrameBuffer::save_frames(winrt::Windows::Storage::StorageFolder storageFolder) 
{
    for (const auto& frame : m_frames) 
    {
        auto file = storageFolder.CreateFileAsync(winrt::to_hstring(frame.filename), winrt::Windows::Storage::CreationCollisionOption::ReplaceExisting).get();

        // Get the file stream
        auto stream = file.OpenAsync(winrt::Windows::Storage::FileAccessMode::ReadWrite).get();

        // Initialize the encoder
        auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(winrt::Windows::Graphics::Imaging::BitmapEncoder::JpegEncoderId(), stream).get();

        // Encode the image
        D3D11_TEXTURE2D_DESC desc = {};
        frame.texture->GetDesc(&desc);
        auto bytes = util::CopyBytesFromTexture(frame.texture);
        encoder.SetPixelData(
            winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
            winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied,
            desc.Width,
            desc.Height,
            1.0,
            1.0,
            bytes);
        encoder.FlushAsync().get();
    }
}
