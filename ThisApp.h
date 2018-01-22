#pragma once

#include <Windows.h>

class IArcBall;

class IThisApp {
public:
	virtual ~IThisApp();

	virtual int Do() = 0;

	virtual std::pair<bool, LRESULT> HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

	static IThisApp* Create(HWND hWnd);
};