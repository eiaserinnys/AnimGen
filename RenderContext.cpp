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

	// ·»´õ Å¸°ÙÀ» ¸¸µç´Ù
	{
		rts.reset(IRenderTargetManager::Create(
			d3d11->g_pd3dDevice, d3d11->g_pSwapChain, d3d11->immDevCtx));

		// ¹é¹öÆÛ Å©±â
		RECT rc;
		int width, height;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		// ±×¸²ÀÚ ¹öÆÛ
		rts->CreateDepthStencilTarget(
			"shadow", 
			DXGI_FORMAT_R32_TYPELESS, 
			DXGI_FORMAT_D32_FLOAT, 
			DXGI_FORMAT_R32_FLOAT, 
			2048, 
			2048);

		// DR Å¸°Ù
		{
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
	}

	// ½¦ÀÌ´õ Á¤ÀÇ¸¦ ¸¸µç´Ù
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

	// ·»´õ·¯¸¦ ¸¸µç´Ù
	{
		objRenderer.reset(IObjectRenderer::Create(this));
		deferredRenderer.reset(IDeferredRenderer::Create(this));
	}

	// ¾ÀÀ» ¸¸µç´Ù (ÀÓ½Ã)
	{
		objBuffer.reset(new ObjectBuffer(d3d11->g_pd3dDevice));
		floor.reset(IFloorMesh::Create(0x64808080, 0xff404040));

		DWORD color = 0x0aa0a0ff;

		// ¸Ó¸®
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0, 1.665f, 0), XMFLOAT3(0.22f, 0.25f, 0.22f), color));

		// ¸ö
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0, 1.22f, 0), XMFLOAT3(0.4f, 0.61f, 0.25f), color));

		// ¿À¸¥ÆÈ
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0.265f, 1.355f, 0), XMFLOAT3(0.11f, 0.35f, 0.11f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0.265f, 0.995f, 0), XMFLOAT3(0.11f, 0.35f, 0.11f), color));

		// ¿ÞÆÈ
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(- 0.265f, 1.355f, 0), XMFLOAT3(0.11f, 0.35f, 0.11f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(- 0.265f, 0.995f, 0), XMFLOAT3(0.11f, 0.35f, 0.11f), color));

		// ¿À¸¥´Ù¸®
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0.115f, 0.71f, 0), XMFLOAT3(0.17f, 0.4f, 0.17f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0.1f, 0.3f, 0), XMFLOAT3(0.14f, 0.4f, 0.14f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(0.1f, 0.045f, -0.05f), XMFLOAT3(0.14f, 0.09f, 0.25f), color));

		// ¿Þ´Ù¸®
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(-0.115f, 0.71f, 0), XMFLOAT3(0.17f, 0.4f, 0.17f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(-0.1f, 0.3f, 0), XMFLOAT3(0.14f, 0.4f, 0.14f), color));
		robot.push_back(IBoxMesh::Create(
			XMFLOAT3(-0.1f, 0.045f, -0.05f), XMFLOAT3(0.14f, 0.09f, 0.25f), color));

		// È­»ìÇ¥µµ Å×½ºÆ®
		XMMATRIX id = XMMatrixIdentity();
		robot.push_back(ICoordinateAxisMesh::Create(id, 3, 0.3f, 0.02f, 0.04f, 8));

		XMMATRIX r = 
			XMMatrixRotationY(30.0f / 180.0f * 3.1415) *
			XMMatrixTranslation(1, 1, 1);

		robot.push_back(ICoordinateAxisMesh::Create(r, 1, 0.3f, 0.02f, 0.04f, 8));
	}

	textRenderer.reset(ITextRenderer::Create(d3d11->g_pd3dDevice, d3d11->immDevCtx));
}

RenderContext::~RenderContext()
{
	for (size_t i = 0; i < robot.size(); ++i)
	{
		delete robot[i];
	}
	robot.clear();

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

	for (size_t i = 0; i < robot.size(); ++i)
	{
		objBuffer->Fill(robot[i]);
	}

	objBuffer->End(d3d11->immDevCtx);
}

