#include "pch.h"

/// <summary>
/// Main entry point for all Google Test tests
/// Initializes WinRT/COM for tests that need D3D11 and Windows.Graphics.Capture
/// </summary>
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Initialize COM for WinRT (required for D3D11 and capture APIs)
    winrt::init_apartment();
    
    int result = RUN_ALL_TESTS();
    
    // Uninitialize COM
    winrt::uninit_apartment();
    
    return result;
}
