#include "pch.h"
#include "UIRenderer.h"

#include <DX11ConstantBufferT.h>
#include <DX11StateBlocks.h>
#include <ShaderManager.h>
#include <Utility.h>

#include "RenderContext.h"

#include "SceneDescriptor.h"
#include "BasicMeshT.h"

using namespace std;
using namespace DirectX;

typedef BasicMeshT<IUIMesh, XMFLOAT2, UINT16> UIMesh;

//------------------------------------------------------------------------------
class UIRenderer : public IUIRenderer {
public:
	//--------------------------------------------------------------------------
	UIRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;

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
	void Render(const XMFLOAT2& extent)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		blendState->Apply(devCtx);

		if (context->sd->Set("ui"))
		{
			unique_ptr<UIMesh> mesh(new UIMesh);

			posB->UpdateDiscard(devCtx, mesh->Vertices().first, mesh->Vertices().second);
			indB->UpdateDiscard(devCtx, mesh->Indices().first, mesh->Indices().second);

			posB->ApplyVB(devCtx, 0, 0);
			indB->ApplyIB(devCtx, 0);

			devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			devCtx->DrawIndexed((UINT)mesh->Indices().second, 0, 0);
		}
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	unique_ptr<IDX11BlendState> blendState;
	unique_ptr<IDX11SamplerState> shaderSampler;

	std::unique_ptr<IDX11Buffer> posB;
	std::unique_ptr<IDX11Buffer> indB;
};

IUIRenderer::~IUIRenderer()
{}

IUIRenderer* IUIRenderer::Create(RenderContext* context)
{
	return new UIRenderer(context);
}
