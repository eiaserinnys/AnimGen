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

#include "ObjectBuffer.h"

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

		rasterState.reset(IDX11RasterizerState::Create_Default(d3dDev));
		depthState.reset(IDX11DepthStencilState::Create_Default(d3dDev));
		depthStateWire.reset(IDX11DepthStencilState::Create_Always(d3dDev));
		blendState.reset(IDX11BlendState::Create_Default(d3dDev));

		constants.reset(new SimpleConstant(d3dDev, devCtx));
	}

	//--------------------------------------------------------------------------
	void Render(
		const SceneDescriptor& sceneDesc,
		ObjectBuffer* objBuffer)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

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

		// 쉐이더 설정
		context->sd->Set("ObjectBody");

		objBuffer->Draw(context->d3d11->immDevCtx);
	}

	//--------------------------------------------------------------------------
	void RenderShadow(
		const SceneDescriptor& sceneDesc,
		ObjectBuffer* objBuffer)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		context->d3d11->immDevCtx->ClearState();
		context->rts->Restore("shadow");

		auto rt = context->rts->GetCurrent();
		rt->ClearDepthStencil(context->d3d11->immDevCtx, 1, 0);

		pair<XMMATRIX, XMFLOAT4> lightTx = sceneDesc.GetLightTransform();

		constants->Update(
			devCtx, 
			lightTx.first, 
			XMMatrixIdentity(), 
			lightTx.second);

		// 상수들
		rasterState->Apply(devCtx);
		depthState->Apply(devCtx);
		blendState->Apply(devCtx);

		// 쉐이더 설정
		context->sd->Set("ObjectShadow");

		objBuffer->Draw(context->d3d11->immDevCtx);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	unique_ptr<SimpleConstant> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;
};

IObjectRenderer::~IObjectRenderer()
{}

IObjectRenderer* IObjectRenderer::Create(RenderContext* context)
{
	return new ObjectRenderer(context);
}
