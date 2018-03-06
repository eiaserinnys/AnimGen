#include "pch.h"
#include "SolverLog.h"

#include "Log.h"

using namespace std;

class SolverLog : public ISolverLog {
public:
	~SolverLog()
	{
		Close();
	}

	//------------------------------------------------------------------------------
	void Open(
		const string& fitDumpName,
		const string& moveDumpName,
		const string& jacDumpName,
		Log* log)
	{
		Close();

		if (!fitDumpName.empty()) { fitDump.open(fitDumpName, ios::trunc); }
		if (!moveDumpName.empty()) { moveDump.open(moveDumpName, ios::trunc); }
		if (!jacDumpName.empty()) { jacDump.open(jacDumpName, ios::trunc); }
		this->log = log;
	}

	//------------------------------------------------------------------------------
	void Close()
	{
		if (fitDump.is_open()) { fitDump.close(); }
		if (moveDump.is_open()) { moveDump.close(); }
		if (jacDump.is_open()) { jacDump.close(); }
		log = nullptr;
	}

	//------------------------------------------------------------------------------
	bool IsOpen(Channel channel)
	{
		if (channel == Residual) { return fitDump.is_open(); }
		if (channel == Move) { return moveDump.is_open(); }
		if (channel == Jacobian) { return jacDump.is_open(); }
		if (channel == Debug) { return true; }
		if (channel == Console) { return log != nullptr; }
		return false;
	}

	//------------------------------------------------------------------------------
	void ForEachChannel(
		int channelMask,
		const wchar_t* format,
		va_list vaList,
		bool newLine)
	{
		auto Test = [&](int v) { return (channelMask & v) == v; };

		wchar_t buffer[4096];
		bool written = false;

		auto FillBuffer = [&]()
		{
			if (!written && format != nullptr)
			{
				_vsnwprintf_s(buffer, 4096, format, vaList);
				written = true;
			}
		};

		auto DumpToFile = [&](wofstream& file)
		{
			if (file.is_open())
			{
				FillBuffer();
				if (format != nullptr) { file << buffer; }
				if (newLine) { file << endl; }
			}
		};

		if (Test(Residual)) { DumpToFile(fitDump); }
		if (Test(Move)) { DumpToFile(moveDump); }
		if (Test(Jacobian)) { DumpToFile(jacDump); }

		if (Test(Debug))
		{
			FillBuffer();
			if (format != nullptr) { OutputDebugStringW(buffer); }
			if (newLine) { OutputDebugStringW(L"\n"); }
		}

		if (Test(Console) && log != nullptr)
		{
			FillBuffer();
			if (format != nullptr)
			{
				if (newLine)
				{
					log->WriteLine(buffer);
				}
				else
				{
					log->Write(buffer);
				}
			}
			else
			{
				if (newLine)
				{
					log->WriteLine(L"");
				}
				else
				{
					log->Write(L"");
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void Write(int channelMask, const wchar_t* format, ...)
	{
		va_list vaList;
		va_start(vaList, format);
		ForEachChannel(channelMask, format, vaList, false);
		va_end(vaList);
	}

	//------------------------------------------------------------------------------
	void WriteLine(int channelMask, const wchar_t* format, ...)
	{
		va_list vaList;
		va_start(vaList, format);
		ForEachChannel(channelMask, format, vaList, true);
		va_end(vaList);
	}

	//------------------------------------------------------------------------------
	void WriteLine(int channelMask)
	{
		ForEachChannel(channelMask, nullptr, nullptr, true);
	}

private:
	wofstream fitDump;
	wofstream moveDump;
	wofstream jacDump;
	Log* log;
};

class SolverLogDummy : public ISolverLog {
public:
	void Open(
		const string& fitDumpName, 
		const string& moveDumpName, 
		const string& jacDumpName,
		Log* log) {}
	void Close() {}

	bool IsOpen(Channel channel) { return false; }

	void Write(int channelMask, const wchar_t* format, ...) {}
	void WriteLine(int channelMask, const wchar_t* format, ...) {}
	void WriteLine(int channelMask) {}
};

ISolverLog::~ISolverLog()
{}

ISolverLog* ISolverLog::Create()
{
	return new SolverLog();
}

ISolverLog* ISolverLog::CreateDummy()
{
	return new SolverLogDummy();
}
