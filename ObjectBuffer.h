#pragma once

#include <memory>
#include <vector>
#include <DX11Buffer.h>

//------------------------------------------------------------------------------
struct ObjectBuffer
{
	ObjectBuffer(ID3D11Device* d3dDev)
	{
		int vbSize = 10000;
		int ibSize = 50000;

		pos.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

		nor.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

		col.reset(IDX11Buffer::Create_DynamicVB(
			d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));
	}

	//--------------------------------------------------------------------------
	void Begin()
	{
		posB.clear();
		norB.clear();
		clrB.clear();
		indB.clear();
	}

	//--------------------------------------------------------------------------
	void Fill(IMesh* mesh)
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
	void End(ID3D11DeviceContext* devCtx)
	{
		pos->UpdateDiscard(devCtx, &posB[0], (UINT)posB.size());
		nor->UpdateDiscard(devCtx, &norB[0], (UINT)norB.size());
		col->UpdateDiscard(devCtx, &clrB[0], (UINT)clrB.size());
		ind->UpdateDiscard(devCtx, &indB[0], (UINT)indB.size());
	}

	//--------------------------------------------------------------------------
	void Draw(ID3D11DeviceContext* devCtx)
	{
		pos->ApplyVB(devCtx, 0, 0);
		nor->ApplyVB(devCtx, 1, 0);
		col->ApplyVB(devCtx, 2, 0);
		ind->ApplyIB(devCtx, 0);

		devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		devCtx->DrawIndexed((UINT)indB.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	std::unique_ptr<IDX11Buffer> pos;
	std::unique_ptr<IDX11Buffer> nor;
	std::unique_ptr<IDX11Buffer> col;
	std::unique_ptr<IDX11Buffer> ind;

	std::vector<DirectX::XMFLOAT3> posB;
	std::vector<DirectX::XMFLOAT3> norB;
	std::vector<DWORD> clrB;
	std::vector<UINT16> indB;
};
