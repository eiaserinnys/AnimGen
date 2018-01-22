#pragma once

#include <Windows.h>

class IArcBall;

class IThisApp {
public:
	virtual ~IThisApp();

	virtual void Do() = 0;

	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) = 0;

	virtual IArcBall* GetArcBall() = 0;

	static IThisApp* Create(HWND hWnd);
};