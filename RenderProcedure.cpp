#include "pch.h"

#include <stdio.h>

#include <DirectXTex.h>
#include "Vector.h"

#include "GeneralizedCoordinate.h"

#include "RenderProcedure.h"

using namespace std;
using namespace Core;
using namespace DirectX;

class RenderProcedure : public IRenderProcedure {
public:
	RenderContext* context;

	//--------------------------------------------------------------------------
	RenderProcedure(RenderContext* context)
		: context(context)
	{
	}

	//--------------------------------------------------------------------------
	void Begin()
	{
		context->rts->Restore();

		float color[] = { 0.5, 0.75, 1, 1 };
		context->rts->GetCurrent()->Clear(context->d3d11->immDevCtx, color, 1, 0);
	}

	//--------------------------------------------------------------------------
	void End()
	{
		context->d3d11->g_pSwapChain->Present(0, 0);
	}

	//--------------------------------------------------------------------------
	void RenderBoneDiagnostic(int& y)
	{
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

		for (int i = 0; i < COUNT_OF(name); ++i)
		{
			auto expMap = context->robot->GetLocalRotation(name[i]);
			auto quat = context->robot->GetLocalQuaternion(name[i]);
			auto quatV = context->robot->GetLocalQuaternionVerify(name[i]);

			auto diff = Distance(quat, quatV);

			XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);
			if (diff > 0.001) { color = XMFLOAT4(0.5f, 0.5f, 1, 1); }
			if (diff > 0.01) { color = XMFLOAT4(0, 0, 1, 1); }

			context->textRenderer->Enqueue(
				TextToRender(
					XMFLOAT2(50, y = i * 15 + 50),
					Utility::FormatW(
						L"Q(%+.4f,%+.4f,%+.4f,%+.4f) -> E(%+.4f,%+.4f,%+.4f) -> Q(%+.4f,%+.4f,%+.4f,%+.4f)",
						quat.x, quat.y, quat.z, quat.w,
						expMap.x, expMap.y, expMap.z,
						quatV.x, quatV.y, quatV.z, quatV.w),
					color));
		}
	}

	//--------------------------------------------------------------------------
	void RenderGeneralizedCoordinate(int& y)
	{
		auto gc = context->robot->Current();

		y += 15;

		context->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"BP(%.4f,%.4f,%.4f) BP(%.4f,%.4f,%.4f)",
					gc.body.first.x, gc.body.first.y, gc.body.first.y,
					gc.body.second.x, gc.body.second.y, gc.body.second.z),
				XMFLOAT4(1, 1, 1, 1)));

		context->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"LLR1(%.4f,%.4f,%.4f) LLL1(%.4f) "
					L"LLR2(%.4f,%.4f,%.4f) LLL2(%.4f) "
					L"LFR(%.4f,%.4f,%.4f) ",
					gc.leg[0].rot1.x, gc.leg[0].rot1.y, gc.leg[0].rot1.z,
					gc.leg[0].len1,
					gc.leg[0].rot2.x, gc.leg[0].rot2.y, gc.leg[0].rot2.z,
					gc.leg[0].len2,
					gc.leg[0].footRot.x, gc.leg[0].footRot.y, gc.leg[0].footRot.z),
				XMFLOAT4(1, 1, 1, 1)));

		context->textRenderer->Enqueue(
			TextToRender(
				XMFLOAT2(50, y += 15),
				Utility::FormatW(
					L"RLR1(%.4f,%.4f,%.4f) RLL1(%.4f) "
					L"RLR2(%.4f,%.4f,%.4f) RLL2(%.4f) "
					L"RFR(%.4f,%.4f,%.4f) ",
					gc.leg[1].rot1.x, gc.leg[1].rot1.y, gc.leg[1].rot1.z,
					gc.leg[1].len1,
					gc.leg[1].rot2.x, gc.leg[1].rot2.y, gc.leg[1].rot2.z,
					gc.leg[1].len2,
					gc.leg[1].footRot.x, gc.leg[1].footRot.y, gc.leg[1].footRot.z),
				XMFLOAT4(1, 1, 1, 1)));
	}

	//--------------------------------------------------------------------------
	void RenderDiagnostic()
	{
		int y = 0;

		//RenderBoneDiagnostic(y);

		//RenderGeneralizedCoordinate(y);
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		RenderDiagnostic();

		context->FillBuffer();

		Begin();

		context->objRenderer->RenderShadow(sceneDesc, context->objBuffer.get());

		context->objRenderer->Render(sceneDesc, context->objBuffer.get());

		context->diagRenderer->Render(sceneDesc);

		{
			context->d3d11->immDevCtx->ClearState();
			context->rts->Restore();

			context->deferredRenderer->Render(sceneDesc);
		}

		{
			int y = 0;

			for (auto
				it = context->logger->entry.begin();
				it != context->logger->entry.end(); ++it)
			{
				context->textRenderer->Enqueue(TextToRender(
					XMFLOAT2(50.0f, 50.0f + y * 20),
					*it,
					XMFLOAT4(1, 0, 0, 1)));

				y++;
			}

			{
				context->d3d11->immDevCtx->ClearState();
				context->rts->Restore();
				context->uiRenderer->Render(
					sceneDesc,
					XMFLOAT2(1280.0f, 720.0f));
			}

			context->textRenderer->Draw(
				sceneDesc.worldViewProj,
				XMFLOAT2(
				(float)context->rts->GetCurrent()->GetWidth(),
					(float)context->rts->GetCurrent()->GetHeight()));
		}

		End();
	}

#if 0
	//--------------------------------------------------------------------------
	void RenderProcedure::Bake(
		IToRender* render,
		const string& srcTexture,
		const wstring& destTexture)
	{
		RenderTuple tuple;
		tuple.render = render;
		tuple.wireframe = Wireframe::False;
		tuple.flag = RenderFlag::Textured;
		tuple.texture = srcTexture;

		auto flag = BakeFlag::Unwrap;

		Begin();

		//RenderInternal(&tuple, 1, flag, false);

		//End(flag);
		{
			ScratchImage img;

			HRESULT hr = DirectX::CaptureTexture(
				device->g_pd3dDevice,
				device->immDevCtx,
				rts->GetCurrent()->GetTexture(0),
				img);

			if (SUCCEEDED(hr))
			{
				DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), destTexture.c_str());
			}
		}
	}
#endif
};

IRenderProcedure* IRenderProcedure::Create(RenderContext* context)
{ return new RenderProcedure(context); }