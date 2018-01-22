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

		int vbSize = 200;
		int ibSize = 600;

		pos.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(XMFLOAT3), vbSize * sizeof(XMFLOAT3)));

		nor.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(XMFLOAT3), vbSize * sizeof(XMFLOAT3)));

		col.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));

		rasterState.reset(IDX11RasterizerState::Create_CullNone(d3dDev));
		depthState.reset(IDX11DepthStencilState::Create_Default(d3dDev));
		depthStateWire.reset(IDX11DepthStencilState::Create_Always(d3dDev));
		blendState.reset(IDX11BlendState::Create_AlphaBlend(d3dDev));

		constants.reset(new SimpleConstant(d3dDev, devCtx));

		mesh.reset(IFloorMesh::Create());
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		pos->UpdateDiscard(devCtx, mesh->Vertices().first, (UINT)mesh->Vertices().second);
		nor->UpdateDiscard(devCtx, mesh->Normals().first, (UINT)mesh->Normals().second);
		col->UpdateDiscard(devCtx, mesh->Colors().first, (UINT)mesh->Colors().second);
		ind->UpdateDiscard(devCtx, mesh->Indices().first, (UINT)mesh->Indices().second);

		devCtx->ClearState();

		context->d3d11->RestoreRenderTarget();

		constants->Update(
			devCtx, 
			sceneDesc.world, 
			sceneDesc.view, 
			sceneDesc.proj, 
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

		devCtx->DrawIndexed((UINT) mesh->Indices().second, 0, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shader/Object.fx";

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

	unique_ptr<IMesh> mesh;
};

IObjectRenderer::~IObjectRenderer()
{}

IObjectRenderer* IObjectRenderer::Create(RenderContext* context)
{
	return new ObjectRenderer(context);
}
