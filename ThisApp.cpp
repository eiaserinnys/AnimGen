#include "pch.h"
#include "ThisApp.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <timeapi.h>

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>
#include <EulerControl.h>

#include "RenderContext.h"
#include "RenderProcedure.h"

#include "IKPicking.h"

using namespace std;
using namespace Core;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<RenderProcedure> render;
	HWND hWnd;

	unique_ptr<IEulerControl> arcBall;
	unique_ptr<IIKPicking> ikPicking;

	bool init = false;

	bool advance = false;

	//--------------------------------------------------------------------------
	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new RenderProcedure(global.get()));

		arcBall.reset(IEulerControl::Create(
			210 / 180.0f * (float)M_PI, 
			20 / 180.0f * (float)M_PI));

		ikPicking.reset(IIKPicking::Create());
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

		XMFLOAT3 target = XMFLOAT3(0, 1.25f, 0);
		arcBall->Update(target, 10);

		sceneDesc.Build(
			hWnd, 
			arcBall->GetEyePosition(), 
			target, 
			arcBall->GetRotationMatrix());

		static DWORD lastTime = 0;
		DWORD cur = timeGetTime();
		DWORD elapsed = lastTime > 0 ? cur - lastTime : 0;
		lastTime = cur;

		if (advance)
		{
			global->robot->Animate_Test(elapsed);
		}

		ikPicking->Update(global->robot.get(), sceneDesc, global.get());

		global->robot->Update();

		static const char* name[] =
		{
			"Body",
			"LLeg1", 
			"LLeg2",
			"LFoot",
			"RLeg1",
			"RLeg2",
			"RFoot",
		};

		int y = 0;
		
		for (int i = 0; i < COUNT_OF(name); ++i)
		{
			auto expMap = global->robot->GetLocalRotation(name[i]);
			auto quat = global->robot->GetLocalQuaternion(name[i]);
			auto quatV = global->robot->GetLocalQuaternionVerify(name[i]);

			auto diff = Distance(quat, quatV);

			XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);
			if (diff > 0.001) { color = XMFLOAT4(0.5f, 0.5f, 1, 1); }
			if (diff > 0.01) { color = XMFLOAT4(0, 0, 1, 1); }

			global->textRenderer->Enqueue(
				TextToRender(
					XMFLOAT2(50, y = i * 15 + 50),
					Utility::FormatW(
						L"Q(%+.4f,%+.4f,%+.4f,%+.4f) -> E(%+.4f,%+.4f,%+.4f) -> Q(%+.4f,%+.4f,%+.4f,%+.4f)",
						quat.x, quat.y, quat.z, quat.w,
						expMap.x, expMap.y, expMap.z,
						quatV.x, quatV.y, quatV.z, quatV.w),
					color));
		}

		auto gc = global->robot->Current();

		y += 15;

		global->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"bp(%.4f,%.4f,%.4f) br(%.4f,%.4f,%.4f)",
					gc.bodyPos.x, gc.bodyPos.y, gc.bodyPos.y,
					gc.bodyRot.x, gc.bodyRot.y, gc.bodyRot.z),
				XMFLOAT4(1, 1, 1, 1)));

		global->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"LLR1(%.4f,%.4f,%.4f) LLL1(%.4f) "
					L"LLR2(%.4f,%.4f,%.4f) LLL2(%.4f) ",
					gc.leg1Rot[0].x, gc.leg1Rot[0].y, gc.leg1Rot[0].z,
					gc.leg1Len[0],
					gc.leg2Rot[0].x, gc.leg2Rot[0].y, gc.leg2Rot[0].z,
					gc.leg2Len[0]),
				XMFLOAT4(1, 1, 1, 1)));

		global->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"RLR1(%.4f,%.4f,%.4f) RLL1(%.4f) "
					L"RLR2(%.4f,%.4f,%.4f) RLL2(%.4f) ",
					gc.leg1Rot[1].x, gc.leg1Rot[1].y, gc.leg1Rot[1].z,
					gc.leg1Len[1],
					gc.leg2Rot[1].x, gc.leg2Rot[1].y, gc.leg2Rot[1].z,
					gc.leg2Len[1]),
				XMFLOAT4(1, 1, 1, 1)));

		global->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"LFR(%.4f,%.4f,%.4f) "
					L"RFR(%.4f,%.4f,%.4f)",
					gc.footRot[0].x, gc.footRot[0].y, gc.footRot[0].z,
					gc.footRot[1].x, gc.footRot[1].y, gc.footRot[1].z),
				XMFLOAT4(1, 1, 1, 1)));

		global->FillBuffer();

		render->Render(sceneDesc);
	}

	//--------------------------------------------------------------------------
	virtual pair<bool, LRESULT> HandleMessage(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		{
			auto result = ikPicking->HandleMessages(hWnd, message, wParam, lParam);
			if (result) { return make_pair(true, result); }
		}

		{
			auto result = arcBall->HandleMessages(hWnd, message, wParam, lParam);
			if (result) { return make_pair(true, result); }
		}

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
		if (wParam == VK_SPACE)
		{
			advance = !advance;
		}
	}
};

IThisApp::~IThisApp()
{}

IThisApp* IThisApp::Create(HWND hWnd)
{
	return new ThisApp(hWnd);
}