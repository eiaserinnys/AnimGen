#include "pch.h"
#include "DeferredRenderer.h"

#include <DX11ConstantBufferT.h>
#include <DX11StateBlocks.h>
#include <ShaderManager.h>
#include <Utility.h>

#include "RenderContext.h"

#include "SceneDescriptor.h"
#include "Mesh.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
struct DeferredConstantsBody
{
	XMFLOAT4 MainLightDir;
	XMFLOAT4 RenderTargetExtent;
	XMMATRIX InvWorldViewProj;
	XMMATRIX LightViewProj;
	XMFLOAT4 ShadowExtent;
	XMFLOAT4 EyePosition;
};

//------------------------------------------------------------------------------
struct DeferredConstants
	: public DX11ConstantBufferT<DeferredConstantsBody>
{
	typedef DX11ConstantBufferT<DeferredConstantsBody> Parent;

	DeferredConstants(
		ID3D11Device* d3dDev,
		ID3D11DeviceContext* devCtx)
		: Parent(d3dDev, devCtx) {}

	void Update(
		ID3D11DeviceContext* devCtx,
		const SceneDescriptor& sceneDesc, 
		int width,
		int height)
	{
		XMMATRIX wvT = XMMatrixTranspose(sceneDesc.invWorldViewT);

		cbChangesEveryFrameMem.MainLightDir.x = sceneDesc.lightDir.x;
		cbChangesEveryFrameMem.MainLightDir.y = sceneDesc.lightDir.y;
		cbChangesEveryFrameMem.MainLightDir.z = sceneDesc.lightDir.z;
		cbChangesEveryFrameMem.MainLightDir.w = 0;

		cbChangesEveryFrameMem.RenderTargetExtent.x = width;
		cbChangesEveryFrameMem.RenderTargetExtent.y = height;
		cbChangesEveryFrameMem.RenderTargetExtent.z = 0.1f;		// zn
		cbChangesEveryFrameMem.RenderTargetExtent.w = 20.f;		// zf

		cbChangesEveryFrameMem.InvWorldViewProj = XMMatrixInverse(nullptr, sceneDesc.worldViewProj);

		auto lightTx = sceneDesc.GetLightTransform();
		cbChangesEveryFrameMem.LightViewProj = lightTx.first;
		
		cbChangesEveryFrameMem.ShadowExtent.x = 2048;
		cbChangesEveryFrameMem.ShadowExtent.y = 2048;
		cbChangesEveryFrameMem.ShadowExtent.z = 0;
		cbChangesEveryFrameMem.ShadowExtent.w = 0;

		cbChangesEveryFrameMem.EyePosition = sceneDesc.eye;

		UpdateInternal(devCtx);
	}
};

//------------------------------------------------------------------------------
class DeferredRenderer : public IDeferredRenderer {
public:
	//--------------------------------------------------------------------------
	DeferredRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		context->sd->Load("deferred", fxFileName, nullptr, 0);

		constants.reset(new DeferredConstants(d3dDev, devCtx));

		blendState.reset(IDX11BlendState::Create_AlphaBlend(d3dDev));
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		constants->Update(devCtx, sceneDesc, 1280, 720);

		blendState->Apply(devCtx);

		context->sd->Set("deferred");

		{
			auto rt = context->rts->GetRenderTarget("deferred");
			ID3D11ShaderResourceView* view[] = 
			{ 
				rt->GetShaderResourceView(0),
				rt->GetShaderResourceView(1),
				rt->GetShaderResourceView(2),
				rt->GetShaderResourceView(3)
			};
			context->d3d11->immDevCtx->PSSetShaderResources(0, 4, view);
		}

		{
			ID3D11ShaderResourceView* view[] =
			{
				context->rts->GetRenderTarget("shadow")->GetShaderResourceView(0), 
			};
			context->d3d11->immDevCtx->PSSetShaderResources(4, 1, view);
		}

		context->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->d3d11->immDevCtx->Draw(3, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shader/Deferred.fx";

	unique_ptr<DeferredConstants> constants;

	unique_ptr<IDX11BlendState> blendState;
};

IDeferredRenderer::~IDeferredRenderer()
{}

IDeferredRenderer* IDeferredRenderer::Create(RenderContext* context)
{
	return new DeferredRenderer(context);
}
