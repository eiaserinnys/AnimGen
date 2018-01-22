#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>
#include <EulerControl.h>

#include "RenderContext.h"
#include "Render.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	unique_ptr<IEulerControl> arcBall;

	bool init = false;

	//--------------------------------------------------------------------------
	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get(), global->rts.get()));

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
		render->Begin();

		SceneDescriptor sceneDesc;

		arcBall->Update(XMFLOAT3(0, 1.5f, 0), 10);

		sceneDesc.Build(
			hWnd, 
			arcBall->GetEyePosition(), 
			arcBall->GetRotationMatrix());

		global->objRenderer->Render(sceneDesc);

		//{
		//	float dist = 1.0f;
		//	XMFLOAT3 ofs = XMFLOAT3(0.5, 0, -0.1f);
		//	sceneDesc.Build(hWnd, com + XMFLOAT3(0.5, 0, -dist) + ofs, com + ofs, rot);
		//	openPoseRender->Render(render.get(), frames[cur], sceneDesc);

		//	render->RenderText(sceneDesc.worldViewProj);
		//}

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