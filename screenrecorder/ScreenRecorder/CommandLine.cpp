#include "pch.h"
#include "CommandLine.h"

std::map<std::string, CommandType> map = { {"-start", CommandType::Start}, 
	{"-stop", CommandType::Stop}, 
	{"-cancel", CommandType::Cancel}, 
	{"-newserver", CommandType::NewServer},
	{"-help", CommandType::Help} };

CommandType CommandLine::GetCommandType() const
{
	if (m_argc < 2) 
	{
		return CommandType::Unknown;
	}

	auto it = map.find(m_argv[1]);

	if (it != map.end()) 
	{
		CommandType type = it->second;
		return type;
	}
	else 
	{
		return CommandType::Unknown;
	}
}

void CommandLine::GetStartArgs(int& framerate, int& monitor, int& bufferSize, bool& isMegabytes) const
{
	int i = 2;
	framerate = 1;
	monitor = 0;
	bufferSize = 100;
	isMegabytes = true;

	while (i < m_argc) 
	{
		if (strcmp(m_argv[i], "-framerate") == 0)
		{
			i++;

			if (i == m_argc)
			{
				throw std::invalid_argument("Syntax error parsing args.");
			}
			
			framerate = std::stoi(m_argv[i]);
			
			i++;
		}
		else if (strcmp(m_argv[i], "-monitor") == 0)
		{
			i++;

			if (i == m_argc)
			{
				throw std::invalid_argument("Syntax error parsing args.");
			}

			monitor = std::stoi(m_argv[i]);

			i++;
		}
		else if (strcmp(m_argv[i], "-framebuffer") == 0)
		{
			i++;

			if (i == m_argc)
			{
				throw std::invalid_argument("Syntax error parsing args.");
			}

			isMegabytes = strcmp(m_argv[i], "-mb") == 0;

			if (isMegabytes)
			{
				i++;
			}

			bufferSize = std::stoi(m_argv[i]);

			i++;
		}
		else
		{
			throw std::invalid_argument("Syntax error parsing args.");
		}
	}
}

void CommandLine::GetStopArgs(std::string& folder) const
{
	if (m_argc < 3)
	{
		throw std::invalid_argument("Syntax error parsing args.");
	}

	folder = m_argv[2];
}

void CommandLine::GetHelpArgs(std::string& arg) const
{
	if (m_argc < 3)
	{
		throw std::invalid_argument("Syntax error parsing args.");
	}

	arg = m_argv[2];
}
