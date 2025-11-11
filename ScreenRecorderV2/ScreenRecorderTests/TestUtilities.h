#pragma once
#include "pch.h"

namespace ScreenRecorder::Testing {

/// <summary>
/// Test utilities for creating mock D3D11 resources
/// </summary>
class TestUtilities {
public:
    /// <summary>
    /// Initialize Direct3D device for testing
    /// </summary>
    static void InitializeD3D();

    /// <summary>
    /// Get the test D3D11 device
    /// </summary>
    static ID3D11Device* GetD3DDevice();

    /// <summary>
    /// Get the test D3D11 device context
    /// </summary>
    static ID3D11DeviceContext* GetD3DDeviceContext();

    /// <summary>
    /// Create a mock texture for testing
    /// </summary>
    /// <param name="width">Texture width</param>
    /// <param name="height">Texture height</param>
    /// <param name="format">DXGI format</param>
    /// <returns>Mock texture</returns>
    static winrt::com_ptr<ID3D11Texture2D> CreateMockTexture(
        UINT width = 1920,
        UINT height = 1080,
        DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM
    );

    /// <summary>
    /// Calculate expected size of a texture in bytes
    /// </summary>
    static size_t CalculateTextureSize(UINT width, UINT height, DXGI_FORMAT format);

    /// <summary>
    /// Create a temporary directory for testing
    /// </summary>
    static std::filesystem::path CreateTempDirectory();

    /// <summary>
    /// Clean up temporary directory
    /// </summary>
    static void CleanupTempDirectory(const std::filesystem::path& path);

private:
    static winrt::com_ptr<ID3D11Device> s_device;
    static winrt::com_ptr<ID3D11DeviceContext> s_deviceContext;
    static bool s_initialized;
};

} // namespace ScreenRecorder::Testing
