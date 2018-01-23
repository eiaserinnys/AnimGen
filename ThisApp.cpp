#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>
#include <EulerControl.h>

#include "RenderContext.h"
#include "RenderProcedure.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<RenderProcedure> render;
	HWND hWnd;

	unique_ptr<IEulerControl> arcBall;

	bool init = false;

	//--------------------------------------------------------------------------
	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new RenderProcedure(hWnd, global->d3d11.get(), global->rts.get()));

		arcBall.reset(IEulerControl::Create());
	}

	//--------------------------------------------------------------------------
	~ThisApp()
	{
		CleanUp();
	}

	//--------------------------------------------------------------------------
	void CleanUp()
	{
		render.reset(NULL);
		global.reset(NULL);
	}

	//--------------------------------------------------------------------------
	virtual int Do()
	{
		return WindowsUtility::MessagePump([&] { DoInternal(); });
	}

	//--------------------------------------------------------------------------
	void DoInternal()
	{
		SceneDescriptor sceneDesc;

		arcBall->Update(XMFLOAT3(0, 1.5f, 0), 10);

		sceneDesc.Build(
			hWnd, 
			arcBall->GetEyePosition(), 
			arcBall->GetRotationMatrix());

		render->Begin();

		global->objRenderer->Render(sceneDesc);

		{
			global->d3d11->immDevCtx->ClearState();
			global->rts->Restore();

			global->deferredRenderer->Render(); 
		}

		render->End();
	}

	//--------------------------------------------------------------------------
	virtual pair<bool, LRESULT> HandleMessage(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto result = arcBall->HandleMessages(hWnd, message, wParam, lParam);
		if (result) { return make_pair(true, result); }

		switch (message) {
		case WM_KEYDOWN:
			OnKeyDown(wParam, lParam);
			return make_pair(true, 0);
		}

		return make_pair(false, 0);
	}

	//--------------------------------------------------------------------------
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam)
	{
		if (wParam == VK_F5)
		{
			try
			{
				global->Reload();
			}
			catch (HRESULT)
			{
				MessageBox(hWnd, L"Reload Error", L"Reload Error", MB_OK);
			}
		}
		if (wParam == VK_F1)
		{
		}
	}
};

IThisApp::~IThisApp()
{}

IThisApp* IThisApp::Create(HWND hWnd)
{
	return new ThisApp(hWnd);
}