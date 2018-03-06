#pragma once

#include <fstream>

struct Log;

class ISolverLog {
public:
	enum Channel
	{
		Residual	= 1 << 0,			// to fit log file
		Move		= 1 << 1,		// to move log file
		Jacobian	= 1 << 2,		// to move log file
		Debug		= 1 << 3,		// to debug dump
		Console		= 1 << 4,		// to console dump
	};

	virtual ~ISolverLog();

	virtual void Open(
		const std::string& fitDumpName, 
		const std::string& moveDumpName, 
		const std::string& jacobianDumpName,
		Log* log) = 0;
	virtual bool IsOpen(Channel channel) = 0;
	virtual void Close() = 0;

	virtual void Write(int channelMask, const wchar_t* format, ...) = 0;
	virtual void WriteLine(int channelMask, const wchar_t* format, ...) = 0;
	virtual void WriteLine(int channelMask) = 0;

	static ISolverLog* Create();
	static ISolverLog* CreateDummy();
};