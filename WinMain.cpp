#include "pch.h"

#include <functional>
#include <memory>

#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <WindowsUtility.h>
#include <ArcBall.h>

#include "ThisApp.h"

#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

static unique_ptr<IThisApp> thisApp;

//------------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	if (thisApp.get() != nullptr)
	{
		auto result = thisApp->GetArcBall()->HandleMessages(hWnd, message, wParam, lParam);
		if (result) { return result; }
	}

	switch (message)
	{
	case WM_KEYDOWN:
		thisApp->OnKeyDown(wParam, lParam);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//------------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	auto ret = WindowsUtility::RegisterAndCreateOverlappedWindow(
		hInstance,
		L"RendererWndClass",
		L"Renderer",
		640 * 2,
		360 * 2,
		nCmdShow,
		WndProc);

	if (FAILED(ret.first)) { return 0; }

	try
	{
		thisApp.reset(IThisApp::Create(ret.second));
	}
	catch (...)
	{
		return 0;
	}

	auto exitCode = WindowsUtility::MessagePump([&] { thisApp->Do(); });

	thisApp.reset(nullptr);

	return exitCode;
}


