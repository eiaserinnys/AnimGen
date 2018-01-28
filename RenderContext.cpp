#include "pch.h"
#include "RenderContext.h"

#include <Utility.h>

#include "AngleHelper.h"
#include "FrameHelper.h"

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

	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		sd->Load("UI", L"Shader/UI.fx", layout, COUNT_OF(layout));
	}
	}

	// 텍스처 매니저를 만든다
	textures.reset(ITextureManager::Create(d3d11->g_pd3dDevice));

	// 렌더러를 만든다
	{
		objRenderer.reset(IObjectRenderer::Create(this));
		deferredRenderer.reset(IDeferredRenderer::Create(this));
		uiRenderer.reset(IUIRenderer::Create(this));
	}

	// 씬을 만든다 (임시)
	{
		objBuffer.reset(new ObjectBuffer(d3d11->g_pd3dDevice));
		floor.reset(IFloorMesh::Create(0x64808080, 0xff404040));

		robot.reset(IRobot::Create());

		// 화살표도 테스트
		XMMATRIX id = XMMatrixIdentity();
		//etc.push_back(ICoordinateAxisMesh::Create(id, 0.5, 0.1f, 0.02f, 0.04f, 8));
	}

	BuildIKRefSphere();

	textRenderer.reset(ITextRenderer::Create(d3d11->g_pd3dDevice, d3d11->immDevCtx));
}

void RenderContext::BuildIKRefSphere()
{
	float innerRadius = 1.25f;
	float radius = 1.5f;
	float tipLen = 0.2f;
	XMFLOAT3 center(0, 1.51f, 0);

	XMMATRIX cTx = XMMatrixTranslation(center.x, center.y, center.z);
	FrameHelper::SetX(cTx, XMFLOAT3(0, -1, 0));
	FrameHelper::SetY(cTx, XMFLOAT3(1, 0, 0));
	FrameHelper::SetZ(cTx, XMFLOAT3(0, 0, 1));
	etc.push_back(ICoordinateAxisMesh::Create(cTx, 1.0f, 0.1f, 0.02f, 0.04f, 8));

	int arrowGranulity = 4;
	float granulity = 11;

	{
		float angle[] = 
		{
			0, -22.5, -45, -56.25, -67.5, -78.75
		};

		for (int a = 0; a < COUNT_OF(angle); ++a)
		{
			float curAngle = angle[a];

			auto angleY = AngleHelperF::DegreeToRadian(curAngle);

			XMMATRIX rotY = XMMatrixRotationY(
				AngleHelperF::DegreeToRadian(curAngle));

			float h1Factor = curAngle < -45 ? 1 - (90 + curAngle) / 45.0f: 0;

			//for (int i = -granulity + 1; i <= granulity - 1; ++i)
			for (int i = 0; i <= granulity - 1; ++i)
			{
				float angle = i / granulity * (float)M_PI;

				// 먼저 좌표를 구한 다음
				XMFLOAT3 legDirT = XMFLOAT3(
					sinf(angle) * cosf(angleY),
					-cosf(angle),
					-sinf(angle) * sinf(angleY)
				);

				// 좌표에서 다시 각도를 구해서 각도를 기준으로 처리하자
				// (실제로 IK 처리 시에는 좌표만 주어질 것이므로)

				XMFLOAT3 footDir = TransformNormal(
					XMFLOAT3(cosf(angle), sinf(angle), 0), rotY);

				XMFLOAT3 footDirH;
				XMStoreFloat3(
					&footDirH,
					XMVectorLerp(
						XMLoadFloat3(&footDir),
						XMLoadFloat3(&XMFLOAT3(1, 0, 0)),
						h1Factor));
				footDirH = Normalize(footDirH);

				etc.push_back(IArrowMesh::Create(
					center + legDirT * innerRadius,
					center + legDirT * radius,
					0.1f,
					0.0125f,
					0.025f,
					arrowGranulity,
					0xff0000ff));

				etc.push_back(IArrowMesh::Create(
					center + legDirT * radius,
					center + legDirT * radius + footDirH * tipLen,
					0.1f,
					0.0125f,
					0.025f,
					arrowGranulity,
					0xff00ff00));
			}
		}
	}

#if 0
	{
		XMMATRIX rotY = XMMatrixRotationY(AngleHelperF::DegreeToRadian(45));
		for (int i = -granulity + 1; i <= granulity - 1; ++i)
		{
			float angle = i / granulity * (float)M_PI;

			XMFLOAT3 legDir = XMFLOAT3(sinf(angle), cosf(angle + (float)M_PI), 0);

			XMFLOAT3 legDirT = TransformNormal(legDir, rotY);

			XMFLOAT3 footDir = TransformNormal(
				XMFLOAT3(cosf(angle), sinf(angle), 0), rotY);

			etc.push_back(IArrowMesh::Create(
				center + legDirT * innerRadius,
				center + legDirT * radius,
				0.1f,
				0.0125f,
				0.025f,
				arrowGranulity,
				0xff0000ff));

			etc.push_back(IArrowMesh::Create(
				center + legDirT * radius,
				center + legDirT * radius + footDir * tipLen,
				0.1f,
				0.0125f,
				0.025f,
				arrowGranulity,
				0xff00ff00));
		}
	}

	{
		XMMATRIX rotY = XMMatrixRotationY(AngleHelperF::DegreeToRadian(67.5));
		for (int i = -granulity + 1; i <= granulity - 1; ++i)
		{
			float angle = i / granulity * (float)M_PI;

			XMFLOAT3 legDir = XMFLOAT3(sinf(angle), cosf(angle + (float)M_PI), 0);

			XMFLOAT3 legDirT = TransformNormal(legDir, rotY);

			XMFLOAT3 footDir = TransformNormal(
				XMFLOAT3(cosf(angle), sinf(angle), 0), rotY);

			etc.push_back(IArrowMesh::Create(
				center + legDirT * innerRadius,
				center + legDirT * radius,
				0.1f,
				0.0125f,
				0.025f,
				arrowGranulity,
				0xff0000ff));

			etc.push_back(IArrowMesh::Create(
				center + legDirT * radius,
				center + legDirT * radius + footDir * tipLen,
				0.1f,
				0.0125f,
				0.025f,
				arrowGranulity,
				0xff00ff00));
		}
	}
#endif

#if 1
	// Z=0 방향
	XMMATRIX rotY = XMMatrixRotationY(AngleHelperF::DegreeToRadian(90));
	for (int i = -granulity + 1; i <= granulity - 1; ++i)
	{
		float angle = i / granulity * (float)M_PI;

		XMFLOAT3 legDir = XMFLOAT3(sinf(angle), cosf(angle + (float)M_PI), 0);

		XMFLOAT3 legDirT;
		XMStoreFloat3(
			&legDirT,
			XMVector3Transform(XMLoadFloat3(&legDir), rotY));

		XMFLOAT3 footDir = XMFLOAT3(1, 0, 0);

		etc.push_back(IArrowMesh::Create(
			center + legDirT * innerRadius,
			center + legDirT * radius,
			0.1f,
			0.0125f,
			0.025f,
			arrowGranulity,
			0xff0000ff));

		etc.push_back(IArrowMesh::Create(
			center + legDirT * radius,
			center + legDirT * radius + footDir * tipLen,
			0.1f,
			0.0125f,
			0.025f,
			arrowGranulity,
			0xff00ff00));
	}
#endif
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

	//objBuffer->Append(robot.get());

	for (size_t i = 0; i < etc.size(); ++i)
	{
		objBuffer->Append(etc[i]);
	}

	objBuffer->End(d3d11->immDevCtx);
}

