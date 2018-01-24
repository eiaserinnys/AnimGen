#include "pch.h"

#include <stdio.h>

#include <DirectXTex.h>

#include "RenderContext.h"
#include "RenderProcedure.h"

using namespace std;
using namespace DirectX;

IToRender::~IToRender()
{
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
RenderProcedure::RenderProcedure(HWND hwnd, DX11Device* device, IRenderTargetManager* rts)
	: hwnd(hwnd), device(device), rts(rts)
{
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

//------------------------------------------------------------------------------
void RenderProcedure::Begin()
{
	rts->Restore();

	float color[] = { 0.5, 0.75, 1, 1 };
	rts->GetCurrent()->Clear(device->immDevCtx, color, 1, 0);
}

//------------------------------------------------------------------------------
void RenderProcedure::End()
{
	device->g_pSwapChain->Present(0, 0);
}


