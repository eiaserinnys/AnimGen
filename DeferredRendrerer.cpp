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

		changing.MainLightDir.x = sceneDesc.lightDir.x;
		changing.MainLightDir.y = sceneDesc.lightDir.y;
		changing.MainLightDir.z = sceneDesc.lightDir.z;
		changing.MainLightDir.w = 0;

		changing.RenderTargetExtent.x = width;
		changing.RenderTargetExtent.y = height;
		changing.RenderTargetExtent.z = sceneDesc.zRange.x; // zn
		changing.RenderTargetExtent.w = sceneDesc.zRange.y; // zf

		changing.InvWorldViewProj = XMMatrixInverse(nullptr, sceneDesc.worldViewProj);

		auto lightTx = sceneDesc.GetLightTransform();
		changing.LightViewProj = lightTx.first;
		
		changing.ShadowExtent.x = 2048;
		changing.ShadowExtent.y = 2048;
		changing.ShadowExtent.z = 0;
		changing.ShadowExtent.w = 0;

		changing.EyePosition = sceneDesc.eye;

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

		{
			D3D11_SAMPLER_DESC sampDesc;
			IDX11SamplerState::DefaultDesc(sampDesc);
			sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			shaderSampler.reset(IDX11SamplerState::Create(d3dDev, sampDesc));
		}
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
			devCtx->PSSetShaderResources(0, 4, view);
		}

		{
			ID3D11ShaderResourceView* view[] =
			{
				context->rts->GetRenderTarget("shadow")->GetShaderResourceView(0), 
			};
			devCtx->PSSetShaderResources(4, 1, view);

			shaderSampler->Apply(devCtx, 4);
		}

		devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		devCtx->Draw(3, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shader/Deferred.fx";

	unique_ptr<DeferredConstants> constants;

	unique_ptr<IDX11BlendState> blendState;
	unique_ptr<IDX11SamplerState> shaderSampler;
};

IDeferredRenderer::~IDeferredRenderer()
{}

IDeferredRenderer* IDeferredRenderer::Create(RenderContext* context)
{
	return new DeferredRenderer(context);
}
