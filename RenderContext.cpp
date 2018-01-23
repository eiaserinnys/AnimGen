#include "pch.h"
#include "RenderContext.h"

#include "Utility.h"

using namespace std;
using namespace DirectX;

RenderContext::RenderContext(HWND hwnd)
	: hwnd(hwnd)
{
	d3d11.reset(new DX11Device(hwnd));

	if (FAILED(d3d11->hr))
	{
		d3d11.reset(NULL);
		throw E_FAIL;
	}

	rts.reset(IRenderTargetManager::Create(d3d11->g_pd3dDevice, d3d11->g_pSwapChain, d3d11->immDevCtx));
	vs.reset(IVertexShaderManager::Create(d3d11.get()));
	ps.reset(IPixelShaderManager::Create(d3d11.get()));

	{
		RECT rc;
		int width, height;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		DXGI_FORMAT formats[] =
		{
			DXGI_FORMAT_B8G8R8A8_UNORM,	// ÄÃ·¯
			DXGI_FORMAT_B8G8R8A8_UNORM,	// ³ë¸Ö
			DXGI_FORMAT_B8G8R8A8_UNORM,	// ºäº¤ÅÍ
			DXGI_FORMAT_R32_FLOAT,		// µª½º
		};

		rts->CreateGenericRenderTarget(
			"deferred",
			formats,
			COUNT_OF(formats),
			width,
			height);
	}

	objRenderer.reset(IObjectRenderer::Create(this));
	deferredRenderer.reset(IDeferredRenderer::Create(this));
}

RenderContext::~RenderContext()
{
	if (d3d11->immDevCtx) { d3d11->immDevCtx->ClearState(); }

	vs.reset(nullptr);
	ps.reset(nullptr);

	d3d11.reset(NULL);
}

void RenderContext::Reload()
{
	//d3d11->ReloadTexture();
	ReloadShader();
}

void RenderContext::ReloadShader()
{
	//dxr->quadVS.reset(new DX11VertexShader(d3d11->g_pd3dDevice, L"Shaders/DX11FullScreenQuad.fx"));
	//dxr->quadPS.reset(new DX11PixelShader(d3d11->g_pd3dDevice, L"Shaders/DX11FullScreenQuad.fx"));
}


