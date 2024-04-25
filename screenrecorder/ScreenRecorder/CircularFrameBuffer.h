#pragma once

#include "pch.h"

namespace util
{
    using namespace robmikh::common::uwp;
}

class CircularFrameBuffer {
public:
    struct Frame {
        winrt::com_ptr<ID3D11Texture2D> texture;
        std::string filename;
        size_t size;
    };

    CircularFrameBuffer(size_t capacity, bool asMegabytes);

    void add_frame(winrt::com_ptr<ID3D11Texture2D> texture, const std::string& filename);
    void save_frames(winrt::Windows::Storage::StorageFolder storageFolder);

private:
    size_t calculate_frame_size(winrt::com_ptr<ID3D11Texture2D> texture);

    size_t m_capacity;
    bool m_asMegabytes;

    size_t m_memoryUsage;
    std::deque<Frame> m_frames;
};
