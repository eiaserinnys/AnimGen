#pragma once

#include <Utility.h>

#include "BasicMeshT.h"

//------------------------------------------------------------------------------
template <typename BaseClass>
class MeshT : public BasicMeshT<BaseClass, DirectX::XMFLOAT3, UINT16> {
public:
	typedef BasicMeshT<BaseClass, DirectX::XMFLOAT3, UINT16> BasicMesh;

	void AddRectangle(
		const DirectX::XMFLOAT3& c,
		const DirectX::XMFLOAT3& rHalf,
		const DirectX::XMFLOAT3& dHalf,
		const DirectX::XMFLOAT3& n,
		DWORD clr)
	{
		using namespace DirectX;
		
		UINT16 indPtr = (UINT16)pos.size();

		pos.push_back(c - rHalf - dHalf);
		pos.push_back(c + rHalf - dHalf);
		pos.push_back(c - rHalf + dHalf);
		pos.push_back(c + rHalf + dHalf);

		nor.push_back(n);
		nor.push_back(n);
		nor.push_back(n);
		nor.push_back(n);

		col.push_back(clr);
		col.push_back(clr);
		col.push_back(clr);
		col.push_back(clr);

		ind.push_back(indPtr + 0);
		ind.push_back(indPtr + 1);
		ind.push_back(indPtr + 2);

		ind.push_back(indPtr + 1);
		ind.push_back(indPtr + 3);
		ind.push_back(indPtr + 2);
	}

	void AddRectangle(
		const DirectX::XMFLOAT3& lu,
		const DirectX::XMFLOAT3& ru,
		const DirectX::XMFLOAT3& ld,
		const DirectX::XMFLOAT3& rd,
		const DirectX::XMFLOAT3& n,
		DWORD clr)
	{
		UINT16 indPtr = (UINT16) pos.size();

		pos.push_back(lu);
		pos.push_back(ru);
		pos.push_back(ld);
		pos.push_back(rd);

		nor.push_back(n);
		nor.push_back(n);
		nor.push_back(n);
		nor.push_back(n);

		col.push_back(clr);
		col.push_back(clr);
		col.push_back(clr);
		col.push_back(clr);

		ind.push_back(indPtr + 0);
		ind.push_back(indPtr + 2);
		ind.push_back(indPtr + 1);

		ind.push_back(indPtr + 1);
		ind.push_back(indPtr + 2);
		ind.push_back(indPtr + 3);
	}

	void Append(typename BaseClass::MeshType* mesh)
	{
		BasicMesh::Append(mesh);
		AppendStream(col, mesh->Colors());
		AppendStream(nor, mesh->Normals());
	}

	const std::pair<const DirectX::XMFLOAT3*, UINT> Normals() const
	{ 
		return StreamContent(nor); 
	}

	const std::pair<const DWORD*, UINT> Colors() const 
	{ 
		return StreamContent(col); 
	}

protected:
	std::vector<DirectX::XMFLOAT3> nor;
	std::vector<DWORD> col;
};
