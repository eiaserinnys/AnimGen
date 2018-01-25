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

//------------------------------------------------------------------------------
struct UIConstantsBody
{
	XMMATRIX UIProjection;
};

//------------------------------------------------------------------------------
struct UIConstants
	: public DX11ConstantBufferT<UIConstantsBody>
{
	typedef DX11ConstantBufferT<UIConstantsBody> Parent;

	UIConstants(
		ID3D11Device* d3dDev,
		ID3D11DeviceContext* devCtx)
		: Parent(d3dDev, devCtx) {}

	void Update(
		ID3D11DeviceContext* devCtx,
		const XMFLOAT2& extent)
	{
		XMMATRIX proj = XMMatrixIdentity();

		proj.r[0].m128_f32[0] = 1 / extent.x * 2.0f;
		proj.r[3].m128_f32[0] = -1;

		proj.r[1].m128_f32[1] = -1 / extent.y * 2.0f;
		proj.r[3].m128_f32[1] = 1;

		changing.UIProjection = XMMatrixTranspose(proj);

		UpdateInternal(devCtx);
	}
};

//------------------------------------------------------------------------------
class UIRenderer : public IUIRenderer {
public:
	class UIBuffer : public BasicMeshT<IUIMesh, XMFLOAT3, UINT16> {
	public:
		const pair<const DWORD*, UINT> Colors() const { return StreamContent(col); }
		const pair<const XMFLOAT2*, UINT> Textures() const { return StreamContent(tex); }

		void Clear()
		{
			pos.clear();
			tex.clear();
			col.clear();
			ind.clear();
		}

		void Append(typename IUIMesh::MeshType* mesh)
		{
			BasicMeshT<IUIMesh, XMFLOAT3, UINT16>::Append(mesh);
			AppendStream(tex, mesh->Textures());
			AppendStream(col, mesh->Colors());
		}

	protected:
		vector<XMFLOAT2> tex;
		vector<DWORD> col;
	};

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

		{
			int vbSize = 10000;
			int ibSize = 50000;

			posB.reset(IDX11Buffer::Create_DynamicVB(
				d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

			texB.reset(IDX11Buffer::Create_DynamicVB(
				d3dDev, sizeof(DirectX::XMFLOAT2), vbSize * sizeof(DirectX::XMFLOAT2)));

			colB.reset(IDX11Buffer::Create_DynamicVB(
				d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

			indB.reset(IDX11Buffer::Create_DynamicIB(
				d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));
		}

		constants.reset(new UIConstants(d3dDev, context->d3d11->immDevCtx));

		buffer.reset(new UIBuffer);
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc, const XMFLOAT2& extent)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		if (context->sd->Set("UI"))
		{
			buffer->Clear();

			XMMATRIX wvp = XMMatrixTranspose(sceneDesc.worldViewProj);

			for (auto it = quads.begin(); it != quads.end(); ++it)
			{
				auto quad = *it;

				XMFLOAT4 posProj;
				XMStoreFloat4(&posProj, XMVector3Transform(XMLoadFloat3(&quad->pos), wvp));

				posProj.x = (1 + posProj.x / posProj.w) / 2 * extent.x;
				posProj.y = (1 - posProj.y / posProj.w) / 2 * extent.y;

				unique_ptr<IUIMesh> mesh(IUIMesh::CreateBasicQuad(
					XMFLOAT3(posProj.x - 25, posProj.y - 25, quad->zDepth),
					XMFLOAT3(posProj.x + 25, posProj.y + 25, quad->zDepth),
					0xffffffff));

				buffer->Append(mesh.get());

				delete quad;
			}

			quads.clear();

			if (buffer->Vertices().second > 0 &&
				buffer->Indices().second > 0)
			{
				blendState->Apply(devCtx);
				constants->Update(devCtx, extent);

				posB->UpdateDiscard(devCtx, buffer->Vertices().first, buffer->Vertices().second);
				texB->UpdateDiscard(devCtx, buffer->Textures().first, buffer->Textures().second);
				colB->UpdateDiscard(devCtx, buffer->Colors().first, buffer->Colors().second);
				indB->UpdateDiscard(devCtx, buffer->Indices().first, buffer->Indices().second);

				posB->ApplyVB(devCtx, 0, 0);
				texB->ApplyVB(devCtx, 1, 0);
				colB->ApplyVB(devCtx, 2, 0);
				indB->ApplyIB(devCtx, 0);

				auto texture = context->textures->Get("Textures/Circle.tga");

				devCtx->PSSetShaderResources(0, 1, &texture.second);

				devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				devCtx->DrawIndexed((UINT)buffer->Indices().second, 0, 0);
			}
		}
	}

	//--------------------------------------------------------------------------
	void Enqueue(const XMFLOAT3& pos, float zDepth)
	{
		quads.push_back(new Quad{ pos, zDepth });
	}

	struct Quad
	{
		XMFLOAT3 pos;
		float zDepth;
	};

	list<Quad*> quads;

	//--------------------------------------------------------------------------
	RenderContext* context;

	unique_ptr<UIBuffer> buffer;

	unique_ptr<IDX11BlendState> blendState;
	unique_ptr<IDX11SamplerState> shaderSampler;
	unique_ptr<UIConstants> constants;

	unique_ptr<IDX11Buffer> posB;
	unique_ptr<IDX11Buffer> texB;
	unique_ptr<IDX11Buffer> colB;
	unique_ptr<IDX11Buffer> indB;
};

IUIRenderer::~IUIRenderer()
{}

IUIRenderer* IUIRenderer::Create(RenderContext* context)
{
	return new UIRenderer(context);
}
