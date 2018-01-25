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

	// ���� Ÿ���� �����
	{
		rts.reset(IRenderTargetManager::Create(
			d3d11->g_pd3dDevice, d3d11->g_pSwapChain, d3d11->immDevCtx));

		// ����� ũ��
		RECT rc;
		int width, height;
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;

		// �׸��� ����
		rts->CreateDepthStencilTarget(
			"shadow", 
			DXGI_FORMAT_R32_TYPELESS, 
			DXGI_FORMAT_D32_FLOAT, 
			DXGI_FORMAT_R32_FLOAT, 
			2048, 
			2048);

		// DR Ÿ��
		{
			DXGI_FORMAT formats[] =
			{
				DXGI_FORMAT_B8G8R8A8_UNORM,	// �÷�
				DXGI_FORMAT_B8G8R8A8_UNORM,	// ���
				DXGI_FORMAT_B8G8R8A8_UNORM,	// �交��
				DXGI_FORMAT_R32_FLOAT,		// ����
			};

			rts->CreateGenericRenderTarget(
				"deferred",
				formats,
				COUNT_OF(formats),
				width,
				height);
		}
	}

	// ���̴� ���Ǹ� �����
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

		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			sd->Load("UI", L"Shader/UI.fx", layout, COUNT_OF(layout));
		}
	}

	// �������� �����
	{
		objRenderer.reset(IObjectRenderer::Create(this));
		deferredRenderer.reset(IDeferredRenderer::Create(this));
		uiRenderer.reset(IUIRenderer::Create(this));
	}

	// ���� ����� (�ӽ�)
	{
		objBuffer.reset(new ObjectBuffer(d3d11->g_pd3dDevice));
		floor.reset(IFloorMesh::Create(0x64808080, 0xff404040));

		robot.reset(IRobot::Create());

		// ȭ��ǥ�� �׽�Ʈ
		XMMATRIX id = XMMatrixIdentity();
		//etc.push_back(ICoordinateAxisMesh::Create(id, 0.5, 0.1f, 0.02f, 0.04f, 8));

		XMMATRIX r = 
			//XMMatrixRotationY(30.0f / 180.0f * 3.1415) *
			XMMatrixTranslation(-1, 1, -1);
		etc.push_back(ICoordinateAxisMesh::Create(r, 1, 0.3f, 0.02f, 0.04f, 8));
	}

	textRenderer.reset(ITextRenderer::Create(d3d11->g_pd3dDevice, d3d11->immDevCtx));
}

RenderContext::~RenderContext()
{
	for (size_t i = 0; i < etc.size(); ++i)
	{
		delete etc[i];
	}
	etc.clear();

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

	objBuffer->Append(floor.get());

	objBuffer->Append(robot.get());

	for (size_t i = 0; i < etc.size(); ++i)
	{
		objBuffer->Append(etc[i]);
	}

	objBuffer->End(d3d11->immDevCtx);
}

