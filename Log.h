#pragma once

#include <string>
#include <list>

struct Log
{
	Log(int logSize = 25);

	void		Write(const wchar_t* msg);
	void		WriteFormatted(const wchar_t* msg, ...);

	void		WriteLine(const wchar_t* msg);
	void		WriteFormattedLine(const wchar_t* msg, ...);

	void		Push(const std::wstring& msg);

	void		Clear();

	const int logSize;

	std::list<std::wstring> msg;
	int msgCount = 0;
	bool newline = true;
};