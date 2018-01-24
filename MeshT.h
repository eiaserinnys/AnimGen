#pragma once

#include <Utility.h>

//------------------------------------------------------------------------------
template <typename BaseClass>
class MeshT : public BaseClass {
public:
	void AddRectangle(
		const DirectX::XMFLOAT3& c,
		const DirectX::XMFLOAT3& rHalf,
		const DirectX::XMFLOAT3& dHalf,
		const DirectX::XMFLOAT3& n,
		DWORD clr)
	{
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

	void Append(IMesh* mesh)
	{
		pos.reserve(pos.size() + mesh->Vertices().second);
		col.reserve(col.size() + mesh->Colors().second);
		nor.reserve(nor.size() + mesh->Normals().second);
		ind.reserve(ind.size() + mesh->Indices().second);

		UINT16 pivot = pos.size();
		pos.insert(pos.end(), mesh->Vertices().first, mesh->Vertices().first + mesh->Vertices().second);
		nor.insert(nor.end(), mesh->Normals().first, mesh->Normals().first + mesh->Normals().second);
		col.insert(col.end(), mesh->Colors().first, mesh->Colors().first + mesh->Colors().second);

		if (pivot > 0)
		{
			for (int i = 0; i < mesh->Indices().second; ++i)
			{
				ind.push_back(mesh->Indices().first[i] + pivot);
			}
		}
		else
		{
			ind.insert(ind.end(), mesh->Indices().first, mesh->Indices().first + mesh->Indices().second);
		}
	}

	virtual std::pair<const DirectX::XMFLOAT3*, UINT> Vertices() const
		{ return make_pair(pos.empty() ? nullptr : &pos[0], (UINT)pos.size()); }
	virtual std::pair<const DirectX::XMFLOAT3*, UINT> Normals() const
		{ return make_pair(nor.empty() ? nullptr : &nor[0], (UINT)nor.size()); }
	virtual std::pair<const DWORD*, UINT> Colors() const
		{ return make_pair(col.empty() ? nullptr : &col[0], (UINT)col.size()); }
	virtual std::pair<const UINT16*, UINT> Indices() const
		{ return make_pair(ind.empty() ? nullptr : &ind[0], (UINT) ind.size()); }

protected:
	std::vector<DirectX::XMFLOAT3> pos;
	std::vector<DirectX::XMFLOAT3> nor;
	std::vector<DWORD> col;
	std::vector<UINT16> ind;
};
