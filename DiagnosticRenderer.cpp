#include "pch.h"
#include "DiagnosticRenderer.h"

#include <timeapi.h>

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>
#include <SimpleConstantBuffer.h>

#include <Utility.h>
#include <WindowsUtility.h>

#include "RenderContext.h"
#include "FrameHelper.h"

#include "Vector.h"
#include "ExponentialMap.h"
#include "DXMathTransform.h"
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
	void EnqueueFrame(const XMMATRIX& m, float length)
	{
		auto x = FrameHelper::GetX(m);
		auto y = FrameHelper::GetY(m);
		auto z = FrameHelper::GetZ(m);
		auto v = FrameHelper::GetTranslation(m);

		size_t pivot = pos.size();
		pos.push_back(v); pos.push_back(v + x * length);
		pos.push_back(v); pos.push_back(v + y * length);
		pos.push_back(v); pos.push_back(v + z * length);

		col.push_back(0xff0000ff); col.push_back(0xff0000ff);
		col.push_back(0xff00ff00); col.push_back(0xff00ff00);
		col.push_back(0xffff0000); col.push_back(0xffff0000);

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
struct SplineTest
{
	unique_ptr<IHermiteSpline> spline;
	vector<Vector3D> pos;
	vector<Vector3D> rot;
	vector<XMMATRIX> tx;

	vector<XMFLOAT3> sampled;

	DWORD lastTime = 0;
	DWORD elapsed = 0;

	int points = 10;

	SplineTest()
	{
		for (int i = 0; i <= points; ++i)
		{
			auto r = [](float range)->float
			{
				return ((float)rand() / RAND_MAX) * range * 2 - range;
			};

			float range = 3.0;

			Vector3D v(r(range), r(1) + 1.5, r(range));
			Vector4D q = Normalize(Vector4D(r(1), r(1), r(1), r(1)));
			Vector3D e = ExponentialMap::FromQuaternion(q);

			Matrix4D m = DXMathTransform<double>::MatrixRotationQuaternion(q);
			FrameHelper::SetTranslation(m, v);

			pos.push_back(v);
			rot.push_back(e);

			XMMATRIX xm;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					xm.r[i].m128_f32[j] = m.m[i][j];
				}
			}

			tx.push_back(xm);
		}

		spline.reset(IHermiteSpline::Create(pos, rot));

		Sample(20);
	}

	void Sample(int g)
	{
		for (int i = 0; i <= pos.size() * g; ++i)
		{
			auto ret = spline->At((double)i / g);
			auto p = ret.first;
			sampled.push_back(XMFLOAT3(p.x, p.y, p.z));
		}
	}

	void Enqueue(LineBuffer* lineBuffer)
	{
		auto curTime = timeGetTime();
		if (lastTime != 0)
		{
			elapsed += curTime - lastTime;
			lastTime = curTime;
		}
		else
		{
			elapsed = 0;
			lastTime = curTime;
		}

		elapsed = elapsed % 30000;

		double factor = (elapsed / 30000.0) * points;

		for (auto it = tx.begin(); it != tx.end(); ++it)
		{
			lineBuffer->EnqueueFrame(*it, 0.25f);
		}

		{
			auto ret = spline->At(factor);

			auto m = DXMathTransform<double>::MatrixRotationQuaternion(
				ExponentialMap::ToQuaternion(ret.second));
			FrameHelper::SetTranslation(m, ret.first);

			XMMATRIX xm;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					xm.r[i].m128_f32[j] = m.m[i][j];
				}
			}

			lineBuffer->EnqueueFrame(xm, 0.5f);
		}

		lineBuffer->Enqueue(sampled, 0xff00ffff);
	}
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

		test.reset(new SplineTest);
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		{
			lineBuffer->Begin();
			test->Enqueue(lineBuffer.get());
			lineBuffer->End(devCtx);
		}

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
	unique_ptr<SplineTest> test;
};

IDiagnosticRenderer::~IDiagnosticRenderer()
{}

IDiagnosticRenderer* IDiagnosticRenderer::Create(RenderContext* context)
{
	return new DiagnosticRenderer(context);
}
