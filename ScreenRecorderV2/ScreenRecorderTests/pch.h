#pragma once

// Google Test
#include <gtest/gtest.h>

// Standard Library
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <mutex>

// Windows and WinRT
#include <Windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wil/resource.h>

// Project headers (will be included in individual test files as needed)
