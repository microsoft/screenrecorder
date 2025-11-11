#include "pch.h"
#include "TestUtilities.h"

namespace ScreenRecorder::Testing {

winrt::com_ptr<ID3D11Device> TestUtilities::s_device;
winrt::com_ptr<ID3D11DeviceContext> TestUtilities::s_deviceContext;
bool TestUtilities::s_initialized = false;

void TestUtilities::InitializeD3D() {
    if (s_initialized) {
        return;
    }

    // Create D3D11 device
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> deviceContext;
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Driver type
        nullptr,                    // Software rasterizer
        createDeviceFlags,          // Flags
        featureLevels,              // Feature levels
        ARRAYSIZE(featureLevels),   // Feature level count
        D3D11_SDK_VERSION,          // SDK version
        device.put(),               // Device
        &featureLevel,              // Feature level
        deviceContext.put()         // Device context
    );

    if (FAILED(hr)) {
        // Fallback to WARP device if hardware fails
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createDeviceFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            device.put(),
            &featureLevel,
            deviceContext.put()
        );
    }

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create D3D11 device for testing");
    }

    s_device = device;
    s_deviceContext = deviceContext;
    s_initialized = true;
}

ID3D11Device* TestUtilities::GetD3DDevice() {
    InitializeD3D();
    return s_device.get();
}

ID3D11DeviceContext* TestUtilities::GetD3DDeviceContext() {
    InitializeD3D();
    return s_deviceContext.get();
}

winrt::com_ptr<ID3D11Texture2D> TestUtilities::CreateMockTexture(
    UINT width,
    UINT height,
    DXGI_FORMAT format
) {
    InitializeD3D();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    winrt::com_ptr<ID3D11Texture2D> texture;
    HRESULT hr = s_device->CreateTexture2D(&desc, nullptr, texture.put());

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create mock texture");
    }

    return texture;
}

size_t TestUtilities::CalculateTextureSize(UINT width, UINT height, DXGI_FORMAT format) {
    UINT bitsPerPixel = 0;

    switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            bitsPerPixel = 32;
            break;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            bitsPerPixel = 64;
            break;
        case DXGI_FORMAT_R8_UNORM:
            bitsPerPixel = 8;
            break;
        default:
            bitsPerPixel = 32; // Default assumption
            break;
    }

    return static_cast<size_t>(width) * height * (bitsPerPixel / 8);
}

std::filesystem::path TestUtilities::CreateTempDirectory() {
    auto tempPath = std::filesystem::temp_directory_path() / 
        ("screenrecorder_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
    
    std::filesystem::create_directories(tempPath);
    return tempPath;
}

void TestUtilities::CleanupTempDirectory(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

} // namespace ScreenRecorder::Testing
