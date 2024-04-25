#pragma once

#include "pch.h"

enum class CommandType { Start, Stop, Cancel, NewServer, Help, Unknown };

class CommandLine {
public:
    CommandLine(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {}

    CommandType GetCommandType() const;
    void GetStartArgs(int& framerate, int& monitor, int& bufferSize, bool& isMegabytes) const;
    void GetStopArgs(std::string& folder) const;
    void GetHelpArgs(std::string& arg) const;

private:
    int m_argc;
    char** m_argv;
};
