#include "pch.h"
#include "ScreenRecorder.h"
#include "MonitorInfo.h"
#include "SimpleCapture.h"
#include "ScreenRecorderProvider.h"

ScreenRecorder::ScreenRecorder() : isCapturing(false)
{
    TraceLoggingRegister(g_hMyComponentProvider);
}

ScreenRecorder::~ScreenRecorder()
{
    TraceLoggingUnregister(g_hMyComponentProvider);
}

void ScreenRecorder::start(int framerate, int monitor, int bufferCapacity, bool asMegabytes)
{
    if (isCapturing)
    {
        throw std::logic_error("\b\tRecording already started.\n");
    }

    std::vector<MonitorInfo> monitors = MonitorInfo::EnumerateAllMonitors(true);

    if (monitor < 0 || monitors.size() <= monitor)
    {
        throw std::out_of_range("\b\tMonitor out of range.\n");
    }

    CircularFrameBuffer buffer(bufferCapacity, asMegabytes);

    auto d3dDevice = util::CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    auto device = CreateDirect3DDevice(dxgiDevice.get());

    MonitorInfo monitorInfo = monitors[monitor];
    auto item = util::CreateCaptureItemForMonitor(monitorInfo.MonitorHandle);

    m_simpleCapture = std::make_unique<SimpleCapture>(device, item, framerate, buffer);

    m_simpleCapture->StartCapture();
    isCapturing = true;
}

void ScreenRecorder::stop(const std::string& folderPath)
{
    if (!isCapturing)
    {
        throw std::logic_error("\b\tRecording is not started.\n");
    }

    StorageFolder storageFolder = NULL;

    try
    {
        storageFolder = StorageFolder::GetFolderFromPathAsync(winrt::to_hstring(folderPath)).get();
    }
    catch (const winrt::hresult_invalid_argument&)
    {
        throw std::invalid_argument("\b\tCould not open folder \"" + folderPath + "\".\n");
    }
    catch (const winrt::hresult_error&)
    {
        throw std::invalid_argument("\b\tCould not open folder \"" + folderPath + "\".\n");
    }

    m_simpleCapture->CloseAndSave(storageFolder);
    isCapturing = false;
}

void ScreenRecorder::cancel()
{
    if (!isCapturing)
    {
        throw std::logic_error("\b\tRecording is not started.\n");
    }

    m_simpleCapture->Close();
    isCapturing = false;
}
