#include "pch.h"
#include "ObjectRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>
#include <SimpleConstantBuffer.h>

#include <Utility.h>

#include "RenderContext.h"

#include "SceneDescriptor.h"
#include "Mesh.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
class ObjectRenderer : public IObjectRenderer {
public:
	//--------------------------------------------------------------------------
	ObjectRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		context->vs->Load(fxFileName);
		context->ps->Load(fxFileName);

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);

		vertexLayout.reset(IDX11InputLayout::Create(
			d3dDev, layout, numElements, context->vs->Find(fxFileName)->pVSBlob));

		int vbSize = 10000;
		int ibSize = 50000;

		pos.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(XMFLOAT3), vbSize * sizeof(XMFLOAT3)));

		nor.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(XMFLOAT3), vbSize * sizeof(XMFLOAT3)));

		col.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));

		rasterState.reset(IDX11RasterizerState::Create_Default(d3dDev));
		depthState.reset(IDX11DepthStencilState::Create_Default(d3dDev));
		depthStateWire.reset(IDX11DepthStencilState::Create_Always(d3dDev));
		blendState.reset(IDX11BlendState::Create_AlphaBlend(d3dDev));

		constants.reset(new SimpleConstant(d3dDev, devCtx));

		floor.reset(IFloorMesh::Create());
		box.reset(IBoxMesh::Create(XMFLOAT3(0, 1.0f, 0), XMFLOAT3(0.5f, 1.0f, 0.4f), 0xff0000ff));
	}

	//--------------------------------------------------------------------------
	void FillBuffer(IMesh* mesh)
	{
		auto ind = mesh->Indices();

		indB.reserve(indB.size() + ind.second);

		for (UINT i = 0; i < ind.second; ++i)
		{
			indB.push_back((UINT16)(ind.first[i] + posB.size()));
		}

		posB.insert(
			posB.end(), 
			mesh->Vertices().first,
			mesh->Vertices().first + mesh->Vertices().second);

		norB.insert(
			norB.end(),
			mesh->Normals().first,
			mesh->Normals().first + mesh->Normals().second);

		clrB.insert(
			clrB.end(),
			mesh->Colors().first,
			mesh->Colors().first + mesh->Colors().second);
	}

	//--------------------------------------------------------------------------
	void FillBuffer()
	{
		posB.clear();
		norB.clear();
		clrB.clear();
		indB.clear();

		FillBuffer(floor.get());
		FillBuffer(box.get());

		{
			ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;
			pos->UpdateDiscard(devCtx, &posB[0], (UINT)posB.size());
			nor->UpdateDiscard(devCtx, &norB[0], (UINT)norB.size());
			col->UpdateDiscard(devCtx, &clrB[0], (UINT)clrB.size());
			ind->UpdateDiscard(devCtx, &indB[0], (UINT)indB.size());
		}
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		FillBuffer();

		context->d3d11->immDevCtx->ClearState();
		context->rts->Restore("deferred");

		auto rt = context->rts->GetCurrent();
		float color[] = { 0.5, 0.75, 1, 0.0f };
		float depth[] = { 50, 0, 0, 0 };
		rt->ClearRenderTarget(context->d3d11->immDevCtx, 0, color);
		rt->ClearRenderTarget(context->d3d11->immDevCtx, 1, color);
		rt->ClearRenderTarget(context->d3d11->immDevCtx, 2, color);
		rt->ClearRenderTarget(context->d3d11->immDevCtx, 3, depth);
		rt->ClearDepthStencil(context->d3d11->immDevCtx, 1, 0);

		constants->Update(
			devCtx, 
			sceneDesc.worldViewProj, 
			sceneDesc.invWorldViewT, 
			sceneDesc.eye);

		// 상수들
		rasterState->Apply(devCtx);
		depthState->Apply(devCtx);
		blendState->Apply(devCtx);

		// 버텍스 입력 레이아웃
		vertexLayout->Apply(devCtx);

		context->vs->Set(fxFileName);
		context->ps->Set(fxFileName);

		pos->ApplyVB(devCtx, 0, 0);
		nor->ApplyVB(devCtx, 1, 0);
		col->ApplyVB(devCtx, 2, 0);
		ind->ApplyIB(devCtx, 0);

		devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		devCtx->DrawIndexed((UINT) indB.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shader/Object.fx";

	vector<XMFLOAT3> posB;
	vector<XMFLOAT3> norB;
	vector<DWORD> clrB;
	vector<UINT16> indB;

	unique_ptr<IDX11Buffer> pos;
	unique_ptr<IDX11Buffer> nor;
	unique_ptr<IDX11Buffer> col;
	unique_ptr<IDX11Buffer> ind;

	unique_ptr<SimpleConstant> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<IDX11InputLayout> vertexLayout;

	unique_ptr<IMesh> floor;
	unique_ptr<IMesh> box;
};

IObjectRenderer::~IObjectRenderer()
{}

IObjectRenderer* IObjectRenderer::Create(RenderContext* context)
{
	return new ObjectRenderer(context);
}
