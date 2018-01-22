#pragma once

//------------------------------------------------------------------------------
template <typename BaseClass>
class MeshT : public BaseClass {
public:
	void AddRectangle(
		const DirectX::XMFLOAT3& lu,
		const DirectX::XMFLOAT3& ru,
		const DirectX::XMFLOAT3& ld,
		const DirectX::XMFLOAT3& rd,
		const DirectX::XMFLOAT3& n)
	{
		UINT16 indPtr = (UINT16) pos.size();

		pos.push_back(lu);
		pos.push_back(ru);
		pos.push_back(ld);
		pos.push_back(rd);

		nor.push_back(n);

		ind.push_back(indPtr + 0);
		ind.push_back(indPtr + 1);
		ind.push_back(indPtr + 2);

		ind.push_back(indPtr + 1);
		ind.push_back(indPtr + 3);
		ind.push_back(indPtr + 2);
	}

	virtual std::pair<const DirectX::XMFLOAT3*, std::size_t> Vertices() const 
		{ return make_pair(pos.empty() ? nullptr : &pos[0], pos.size()); }
	virtual std::pair<const DirectX::XMFLOAT3*, std::size_t> Normals() const
		{ return make_pair(nor.empty() ? nullptr : &nor[0], nor.size()); }
	virtual std::pair<const UINT16*, std::size_t> Indices() const
		{ return make_pair(ind.empty() ? nullptr : &ind[0], ind.size()); }

protected:
	std::vector<DirectX::XMFLOAT3> pos;
	std::vector<DirectX::XMFLOAT3> nor;
	std::vector<UINT16> ind;
};
