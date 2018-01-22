#include "pch.h"

#include <stdio.h>

#include <DirectXTex.h>

#include "RenderContext.h"
#include "Render.h"

using namespace std;
using namespace DirectX;

IToRender::~IToRender()
{
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
DX11Render::DX11Render(HWND hwnd, DX11Device* device)
	: hwnd(hwnd), device(device)
{
	//lastTime = timeGetTime();

	spriteBatch.reset(new SpriteBatch(device->immDevCtx));
	font.reset(new SpriteFont(device->g_pd3dDevice, L"Font/font12.spritefont"));
}

//------------------------------------------------------------------------------
void DX11Render::Render(RenderTuple* tuples, int count, bool wireframe)
{
	if (tuples != nullptr && count > 0)
	{
		//RenderInternal(tuples, count, BakeFlag::None, wireframe);
	}
}

//------------------------------------------------------------------------------
void DX11Render::Bake(IToRender* render, const string& srcTexture, const wstring& destTexture)
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
			device->GetRenderTarget()->GetTexture(),
			img);

		if (SUCCEEDED(hr))
		{
			DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), destTexture.c_str());
		}
	}
}

//------------------------------------------------------------------------------
void DX11Render::UpsampleDepth(IToRender* render, vector<float>& buffer, bool fhd)
{
	RenderTuple tuple;
	tuple.render = render;
	tuple.wireframe = Wireframe::False;
	tuple.flag = RenderFlag::VertexColor;

	auto flag = fhd ? BakeFlag::DepthUpsample : BakeFlag::WLS;

	Begin(flag);

	//RenderInternal(&tuple, 1, flag, false);

	//End(flag);
	{
		ScratchImage img;

		HRESULT hr = DirectX::CaptureTexture(
			device->g_pd3dDevice,
			device->immDevCtx,
			device->GetRenderTarget()->GetTexture(),
			img);

		if (SUCCEEDED(hr))
		{
			auto image = img.GetImage(0, 0, 0);

			const float* byFloat = (const float*)image->pixels;

			auto width = device->GetRenderTarget()->GetWidth();
			auto height = device->GetRenderTarget()->GetHeight();

			buffer.resize(width * height);

			memcpy(&buffer[0], byFloat, sizeof(buffer[0]) * buffer.size());
		}
	}
}

//------------------------------------------------------------------------------
void DX11Render::Begin(BakeFlag::Value bake)
{
	switch (bake)
	{
	default:
	case BakeFlag::None:
		device->SetScreenshotMode(DX11Device::RenderTarget::Backbuffer);
		break;

	case BakeFlag::Unwrap:
		device->SetScreenshotMode(DX11Device::RenderTarget::ForUnwrap);
		break;

	case BakeFlag::DepthUpsample:
		device->SetScreenshotMode(DX11Device::RenderTarget::ForUpsample);
		break;

	case BakeFlag::WLS:
		device->SetScreenshotMode(DX11Device::RenderTarget::ForWls);
		break;
	}

	device->RestoreRenderTarget();
	device->ClearRenderTarget();
}

//------------------------------------------------------------------------------
void DX11Render::End()
{
	device->g_pSwapChain->Present(0, 0);
}

//------------------------------------------------------------------------------
void DX11Render::RenderText(const XMMATRIX& wvp_)
{
	if (!textToRender.empty())
	{
		device->RestoreRenderTarget();

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
				pos.m128_f32[0] = (1 + pos.m128_f32[0]) / 2 * device->GetRenderTargetWidth();
				pos.m128_f32[1] = (1 - pos.m128_f32[1]) / 2 * device->GetRenderTargetHeight();

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

