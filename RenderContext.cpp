#include "pch.h"
#include "RenderContext.h"

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

	objRenderer.reset(IObjectRenderer::Create(this));

	{
		RECT rc;
		int width, height;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		rts->CreateGenericRenderTarget(
			"deferred", DXGI_FORMAT_B8G8R8A8_UNORM, width, height);
	}

	vs->Load(L"Shader/FullScreenQuad.fx");
	ps->Load(L"Shader/FullScreenQuad.fx");
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


