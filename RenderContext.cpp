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

	sd.reset(IShaderDefineManager::Create(d3d11.get()));

	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		sd->Load("ObjectBody", L"Shader/Object.fx", layout, COUNT_OF(layout));
	}

	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		sd->Load("ObjectShadow", L"Shader/Shadow.fx", layout, COUNT_OF(layout));
	}

	objRenderer.reset(IObjectRenderer::Create(this));
	deferredRenderer.reset(IDeferredRenderer::Create(this));
}

RenderContext::~RenderContext()
{
	if (d3d11->immDevCtx) { d3d11->immDevCtx->ClearState(); }

	sd.reset(nullptr);

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


