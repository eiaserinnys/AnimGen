#pragma once

#include <list>
#include <string>
#include <DX11Shader.h>

class Logger : public IShaderCompileLog {
public:
	void Log(const char* log);

	void Clamp();

	std::list<std::wstring> entry;
};