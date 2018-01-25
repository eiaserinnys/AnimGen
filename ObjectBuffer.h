#pragma once

#include <memory>
#include <vector>
#include <DX11Buffer.h>

#include "Mesh.h"
#include "MeshT.h"

//------------------------------------------------------------------------------
struct ObjectBuffer : public MeshT<IMesh>
{
	ObjectBuffer(ID3D11Device* d3dDev)
	{
		int vbSize = 10000;
		int ibSize = 50000;

		posB.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

		norB.reset(IDX11Buffer::Create_DynamicVB(
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
		nor.clear();
		col.clear();
		ind.clear();
	}

	//--------------------------------------------------------------------------
	void End(ID3D11DeviceContext* devCtx)
	{
		posB->UpdateDiscard(devCtx, &pos[0], (UINT)pos.size());
		norB->UpdateDiscard(devCtx, &nor[0], (UINT)nor.size());
		colB->UpdateDiscard(devCtx, &col[0], (UINT)col.size());
		indB->UpdateDiscard(devCtx, &ind[0], (UINT)ind.size());
	}

	//--------------------------------------------------------------------------
	void Draw(ID3D11DeviceContext* devCtx)
	{
		posB->ApplyVB(devCtx, 0, 0);
		norB->ApplyVB(devCtx, 1, 0);
		colB->ApplyVB(devCtx, 2, 0);
		indB->ApplyIB(devCtx, 0);

		devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		devCtx->DrawIndexed((UINT)ind.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	std::unique_ptr<IDX11Buffer> posB;
	std::unique_ptr<IDX11Buffer> norB;
	std::unique_ptr<IDX11Buffer> colB;
	std::unique_ptr<IDX11Buffer> indB;
};
