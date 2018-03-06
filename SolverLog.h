#pragma once

#include <fstream>

struct Log;

class ISolverLog {
public:
	enum Channel
	{
		Fit = 0x01,			// to fit log file
		Move = 0x02,		// to move log file
		Debug = 0x04,		// to debug dump
		Console = 0x08,		// to console dump
	};

	virtual ~ISolverLog();

	virtual void Open(const std::string& fitDumpName, const std::string& moveDumpName, Log* log) = 0;
	virtual bool IsOpen(Channel channel) = 0;
	virtual void Close() = 0;

	virtual void Write(int channelMask, const wchar_t* format, ...) = 0;
	virtual void WriteLine(int channelMask, const wchar_t* format, ...) = 0;
	virtual void WriteLine(int channelMask) = 0;

	static ISolverLog* Create();
	static ISolverLog* CreateDummy();
};