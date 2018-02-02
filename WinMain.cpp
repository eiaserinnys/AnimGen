#include "pch.h"

#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "winmm.lib")

#include <WindowsUtility.h>

#include "ThisApp.h"

using namespace std;

static unique_ptr<IThisApp> thisApp;

extern void TestVector();

//------------------------------------------------------------------------------
INT WINAPI wWinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPWSTR lpCmdLine, 
	int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	TestVector();
	return 0;

	auto ret = WindowsUtility::RegisterAndCreateOverlappedWindow(
		hInstance,
		L"RendererWndClass",
		L"Renderer",
		640 * 2,
		360 * 2,
		nCmdShow,
		[](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
		{
			if (thisApp.get() != nullptr)
			{
				auto ret = thisApp->HandleMessage(hWnd, message, wParam, lParam);
				if (ret.first) { return ret.second; }
			}
			return WindowsUtility::DefaultWndProc(hWnd, message, wParam, lParam);
		});

	if (FAILED(ret.first)) { return 0; }

	thisApp.reset(IThisApp::Create(ret.second));

	auto exitCode = thisApp->Do();

	thisApp.reset(nullptr);

	return exitCode;
}


