#include "pch.h"
#include "Log.h"

#include <Utility.h>

using namespace std;

#define USE 1
#define DEBUG_OUTPUT 0

Log::Log(int logSize)
	: logSize(logSize)
{}

void Log::Clear()
{
	msg.clear();
	msgCount = 0;
	newline = true;
}

void Log::Write(const wchar_t* msg)
{
#if USE
#if DEBUG_OUTPUT
	DebugOutput(msg);
#endif

	Push(msg);
	newline = false;
#endif
}

void Log::WriteFormatted(const wchar_t* format, ...)
{
#if USE
	wchar_t buffer[4096];
	va_list vaList;
	va_start(vaList, format);
	_vsnwprintf_s(buffer, 4096, format, vaList);
	va_end(vaList);

#if DEBUG_OUTPUT
	DebugOutput(buffer);
#endif

	Push(buffer);
	newline = false;
#endif
}

void Log::WriteLine(const wchar_t* msg)
{
#if USE
#if DEBUG_OUTPUT
	DebugOutput(msg);
	DebugOutput(L"\n");
#endif

	Push(msg);
	newline = true;
#endif
}

void Log::WriteFormattedLine(const wchar_t* format, ...)
{
#if USE
	wchar_t buffer[4096];
	va_list vaList;
	va_start(vaList, format);
	_vsnwprintf_s(buffer, 4096, format, vaList);
	va_end(vaList);

#if DEBUG_OUTPUT
	DebugOutput(buffer);
	DebugOutput(L"\n");
#endif

	Push(buffer);
	newline = true;
#endif
}

void Log::Push(const wstring& text)
{
	if (newline || msg.empty())
	{
		msg.push_back(text);
		msgCount++;
	}
	else
	{
		(*msg.rbegin()) += text;
	}

	while (msgCount > logSize)
	{
		msg.pop_front();
		msgCount--;
	}
}