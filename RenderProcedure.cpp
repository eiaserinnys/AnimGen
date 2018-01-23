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
	//lastTime = timeGetTime();

	spriteBatch.reset(new SpriteBatch(device->immDevCtx));
	font.reset(new SpriteFont(device->g_pd3dDevice, L"Font/font12.spritefont"));
}

//------------------------------------------------------------------------------
void RenderProcedure::Render(RenderTuple* tuples, int count, bool wireframe)
{
	if (tuples != nullptr && count > 0)
	{
		//RenderInternal(tuples, count, BakeFlag::None, wireframe);
	}
}

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

	Begin(flag);

	//RenderInternal(&tuple, 1, flag, false);

	//End(flag);
	{
		ScratchImage img;

		HRESULT hr = DirectX::CaptureTexture(
			device->g_pd3dDevice,
			device->immDevCtx,
			rts->GetCurrent()->GetTexture(),
			img);

		if (SUCCEEDED(hr))
		{
			DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), destTexture.c_str());
		}
	}
}

//------------------------------------------------------------------------------
void RenderProcedure::Begin()
{
	rts->Restore();
	rts->Clear();
}

//------------------------------------------------------------------------------
void RenderProcedure::End()
{
	device->g_pSwapChain->Present(0, 0);
}

//------------------------------------------------------------------------------
void RenderProcedure::RenderText(const XMMATRIX& wvp_)
{
	if (!textToRender.empty())
	{
		rts->Restore();

		spriteBatch->Begin();

		XMMATRIX wvp = XMMatrixTranspose(wvp_);

		for (auto tit = textToRender.begin(); tit != textToRender.end(); ++tit)
		{
			XMVECTOR pos;
			if (tit->is3d)
			{
				pos = XMVector3Transform(XMLoadFloat3(&tit->pos), wvp);

				pos.m128_f32[0] /= pos.m128_f32[3];
				pos.m128_f32[1] /= pos.m128_f32[3];
				pos.m128_f32[2] /= pos.m128_f32[3];
				pos.m128_f32[3] /= pos.m128_f32[3];
				pos.m128_f32[0] = (1 + pos.m128_f32[0]) / 2 * rts->GetWidth();
				pos.m128_f32[1] = (1 - pos.m128_f32[1]) / 2 * rts->GetHeight();

				pos = pos + XMLoadFloat2(&tit->ofs);
			}
			else
			{
				pos = XMLoadFloat2(&tit->ofs);
			}

			font->DrawString(
				spriteBatch.get(),
				tit->text.c_str(),
				pos,
				XMLoadFloat4(&tit->clr));
		}

		spriteBatch->End();

		device->immDevCtx->ClearState();

		textToRender.clear();
	}
}

