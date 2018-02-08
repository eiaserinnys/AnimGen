#include "pch.h"

#include <stdio.h>

#include <DirectXTex.h>

#include "RenderProcedure.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
RenderProcedure::RenderProcedure(RenderContext* context)
	: context(context)
{
}

//------------------------------------------------------------------------------
void RenderProcedure::Begin()
{
	context->rts->Restore();

	float color[] = { 0.5, 0.75, 1, 1 };
	context->rts->GetCurrent()->Clear(context->d3d11->immDevCtx, color, 1, 0);
}

//------------------------------------------------------------------------------
void RenderProcedure::End()
{
	context->d3d11->g_pSwapChain->Present(0, 0);
}

//------------------------------------------------------------------------------
void RenderProcedure::Render(const SceneDescriptor& sceneDesc)
{
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
//------------------------------------------------------------------------------
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


