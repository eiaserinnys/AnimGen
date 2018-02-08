#include "pch.h"
#include "DiagnosticRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>
#include <SimpleConstantBuffer.h>

#include <Utility.h>
#include <WindowsUtility.h>

#include "RenderContext.h"

#include "Vector.h"
#include "SceneDescriptor.h"

#include "HermiteSpline.h"

using namespace std;
using namespace Core;
using namespace DirectX;

//------------------------------------------------------------------------------
struct LineBuffer : public MeshT<IMesh>
{
	LineBuffer(ID3D11Device* d3dDev)
	{
		int vbSize = 10000;
		int ibSize = 50000;

		posB.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

		colB.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

		indB.reset(IDX11Buffer::Create_DynamicIB(
			d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));
	}

	//--------------------------------------------------------------------------
	void Begin()
	{
		pos.clear();
		col.clear();
		ind.clear();
	}

	//--------------------------------------------------------------------------
	void End(ID3D11DeviceContext* devCtx)
	{
		posB->UpdateDiscard(devCtx, &pos[0], (UINT)pos.size());
		colB->UpdateDiscard(devCtx, &col[0], (UINT)col.size());
		indB->UpdateDiscard(devCtx, &ind[0], (UINT)ind.size());
	}

	//--------------------------------------------------------------------------
	void EnqueueAnchor(const XMFLOAT3& v, float length, DWORD c)
	{
		size_t pivot = pos.size();
		pos.push_back(v - XMFLOAT3(length / 2, 0, 0));
		pos.push_back(v + XMFLOAT3(length / 2, 0, 0));
		pos.push_back(v - XMFLOAT3(0, length / 2, 0));
		pos.push_back(v + XMFLOAT3(0, length / 2, 0));
		pos.push_back(v - XMFLOAT3(0, 0, length / 2));
		pos.push_back(v + XMFLOAT3(0, 0, length / 2));

		col.insert(col.end(), 6, c);

		ind.push_back(pivot++); ind.push_back(pivot++);
		ind.push_back(pivot++); ind.push_back(pivot++);
		ind.push_back(pivot++); ind.push_back(pivot++);
	}

	//--------------------------------------------------------------------------
	void Enqueue(vector<XMFLOAT3>& p, DWORD c)
	{
		size_t pivot = pos.size();
		pos.reserve(pos.size() + p.size());
		pos.insert(pos.end(), p.begin(), p.end());

		col.reserve(col.size() + p.size());
		col.insert(col.end(), p.size(), c);

		for (size_t i = 0; i + 1 < p.size(); ++i)
		{
			ind.push_back(i + pivot);
			ind.push_back(i + 1 + pivot);
		}
	}

	//--------------------------------------------------------------------------
	void Draw(ID3D11DeviceContext* devCtx)
	{
		posB->ApplyVB(devCtx, 0, 0);
		colB->ApplyVB(devCtx, 1, 0);
		indB->ApplyIB(devCtx, 0);

		devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		devCtx->DrawIndexed((UINT)ind.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	std::unique_ptr<IDX11Buffer> posB;
	std::unique_ptr<IDX11Buffer> colB;
	std::unique_ptr<IDX11Buffer> indB;

	vector<XMFLOAT3> pos;
	vector<DWORD> col;
	vector<UINT16> ind;
};

//------------------------------------------------------------------------------
class DiagnosticRenderer : public IDiagnosticRenderer {
public:
	//--------------------------------------------------------------------------
	DiagnosticRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		rasterState.reset(IDX11RasterizerState::Create_Default(d3dDev));
		depthState.reset(IDX11DepthStencilState::Create_Default(d3dDev));
		depthStateWire.reset(IDX11DepthStencilState::Create_Always(d3dDev));
		blendState.reset(IDX11BlendState::Create_Default(d3dDev));

		constants.reset(new SimpleConstant(d3dDev, devCtx));

		lineBuffer.reset(new LineBuffer(d3dDev));

		{
			lineBuffer->Begin();

			vector<Vector3D> pd;
			for (int i = 0; i <= 20; ++i)
			{
				auto r = [](float range)->float
				{
					return ((float)rand() / RAND_MAX) * range * 2 - range;
				};

				float range = 2.0;

				Vector3D v(r(range), r(1.5) + 1.5, r(range));

				pd.push_back(v);

				lineBuffer->EnqueueAnchor(XMFLOAT3(v.x, v.y, v.z), 0.05f, 0xff0000ff);
			}

			unique_ptr<IHermiteSpline> spline(IHermiteSpline::Create(pd));

			vector<XMFLOAT3> c;
			int g = 20;
			for (int i = 0; i <= 20 * g; ++i)
			{
				auto v = spline->At((double) i / g);
				c.push_back(XMFLOAT3(v.x, v.y, v.z));
			}

			lineBuffer->Enqueue(c, 0xff00ffff);

			lineBuffer->End(devCtx);
		}
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		context->d3d11->immDevCtx->ClearState();
		context->rts->Restore("deferred");

		constants->Update(
			devCtx,
			sceneDesc.worldViewProj,
			sceneDesc.invWorldViewT,
			sceneDesc.eye);

		// 상수들
		rasterState->Apply(devCtx);
		depthState->Apply(devCtx);
		blendState->Apply(devCtx);

		// 쉐이더 설정
		if (context->sd->Set("Diagnostic"))
		{ 
			lineBuffer->Draw(devCtx);
		}		
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	unique_ptr<SimpleConstant> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<LineBuffer> lineBuffer;
};

IDiagnosticRenderer::~IDiagnosticRenderer()
{}

IDiagnosticRenderer* IDiagnosticRenderer::Create(RenderContext* context)
{
	return new DiagnosticRenderer(context);
}
