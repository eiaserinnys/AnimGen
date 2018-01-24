#include "pch.h"
#include "RenderContext.h"

#include "Utility.h"

using namespace std;
using namespace DirectX;

RenderContext::RenderContext(HWND hwnd)
	: hwnd(hwnd)
{
	logger.reset(new Logger);

	d3d11.reset(new DX11Device(hwnd));

	if (FAILED(d3d11->hr))
	{
		d3d11.reset(NULL);
		throw E_FAIL;
	}

	// 렌더 타겟을 만든다
	{
		rts.reset(IRenderTargetManager::Create(
			d3d11->g_pd3dDevice, d3d11->g_pSwapChain, d3d11->immDevCtx));

		// 백버퍼 크기
		RECT rc;
		int width, height;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		// 그림자 버퍼
		rts->CreateDepthStencilTarget(
			"shadow", 
			DXGI_FORMAT_R32_TYPELESS, 
			DXGI_FORMAT_D32_FLOAT, 
			DXGI_FORMAT_R32_FLOAT, 
			2048, 
			2048);

		// DR 타겟
		{
			DXGI_FORMAT formats[] =
			{
				DXGI_FORMAT_B8G8R8A8_UNORM,	// 컬러
				DXGI_FORMAT_B8G8R8A8_UNORM,	// 노멀
				DXGI_FORMAT_B8G8R8A8_UNORM,	// 뷰벡터
				DXGI_FORMAT_R32_FLOAT,		// 뎁스
			};

			rts->CreateGenericRenderTarget(
				"deferred",
				formats,
				COUNT_OF(formats),
				width,
				height);
		}
	}

	// 쉐이더 정의를 만든다
	{
		sd.reset(IShaderDefineManager::Create(d3d11.get()));
		sd->SetCompileLogger(logger.get());

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
	}

	// 렌더러를 만든다
	{
		objRenderer.reset(IObjectRenderer::Create(this));
		deferredRenderer.reset(IDeferredRenderer::Create(this));
	}

	// 씬을 만든다 (임시)
	{
		objBuffer.reset(new ObjectBuffer(d3d11->g_pd3dDevice));
		floor.reset(IFloorMesh::Create(0x64808080, 0xff404040));
		box.reset(IBoxMesh::Create(XMFLOAT3(0, 1.0f, 0), XMFLOAT3(0.5f, 1.0f, 0.4f), 0x068080ff));
	}

	textRenderer.reset(ITextRenderer::Create(d3d11->g_pd3dDevice, d3d11->immDevCtx));
}

RenderContext::~RenderContext()
{
	if (d3d11->immDevCtx) { d3d11->immDevCtx->ClearState(); }

	sd.reset(nullptr);

	d3d11.reset(NULL);
}

void RenderContext::Reload()
{
	logger->Clear();

	sd->Reload();
}

void RenderContext::FillBuffer()
{
	objBuffer->Begin();

	objBuffer->Fill(floor.get());
	objBuffer->Fill(box.get());

	objBuffer->End(d3d11->immDevCtx);
}

