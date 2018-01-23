#include "pch.h"
#include "DeferredRenderer.h"

#include <DX11ConstantBufferT.h>

#include <Utility.h>

#include "RenderContext.h"

#include "SceneDescriptor.h"
#include "Mesh.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
struct DeferredConstantsBody
{
	DirectX::XMFLOAT4 RenderTargetExtent;
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
		int width,
		int height)
	{
		cbChangesEveryFrameMem.RenderTargetExtent.x = width;
		cbChangesEveryFrameMem.RenderTargetExtent.y = height;
		cbChangesEveryFrameMem.RenderTargetExtent.z = 0;
		cbChangesEveryFrameMem.RenderTargetExtent.w = 0;

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

		context->vs->Load(fxFileName);
		context->ps->Load(fxFileName);

		constants.reset(new DeferredConstants(d3dDev, devCtx));
	}

	//--------------------------------------------------------------------------
	void Render()
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		constants->Update(devCtx, 1280, 720);

		context->vs->Set(fxFileName);
		context->ps->Set(fxFileName);

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

		context->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->d3d11->immDevCtx->Draw(3, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shader/Deferred.fx";

	unique_ptr<DeferredConstants> constants;
};

IDeferredRenderer::~IDeferredRenderer()
{}

IDeferredRenderer* IDeferredRenderer::Create(RenderContext* context)
{
	return new DeferredRenderer(context);
}
