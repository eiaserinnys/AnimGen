#include "pch.h"
#include "Logger.h"

#include "Utility.h"

using namespace std;

void Logger::Log(const char* log)
{
	entry.push_back(Utility::FormatW(L"%S", log));

	Clamp();
}

void Logger::Clamp()
{
	while (entry.size() > 20)
	{
		entry.pop_front();
	}
}
