#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>
#include <ArcBall.h>

#include "RenderContext.h"
#include "Render.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	unique_ptr<IArcBall> arcBall;

	bool init = false;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));

		arcBall.reset(IArcBall::Create());
	}

	~ThisApp()
	{
		CleanUp();
	}

	void CleanUp()
	{
		render.reset(NULL);
		global.reset(NULL);
	}

	IArcBall* GetArcBall() { return arcBall.get(); }

	virtual void Do()
	{
		render->Begin();

		SceneDescriptor sceneDesc;

		XMMATRIX rot = arcBall->GetRotationMatrix();

		//{
		//	float dist = 1.0f;
		//	XMFLOAT3 ofs = XMFLOAT3(0.5, 0, -0.1f);
		//	sceneDesc.Build(hWnd, com + XMFLOAT3(0.5, 0, -dist) + ofs, com + ofs, rot);
		//	openPoseRender->Render(render.get(), frames[cur], sceneDesc);

		//	render->RenderText(sceneDesc.worldViewProj);
		//}

		render->End();
	}

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