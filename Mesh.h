#pragma once

class IMesh {
public:
	virtual ~IMesh();

	virtual std::pair<const DirectX::XMFLOAT3*, UINT> Vertices() const = 0;
	virtual std::pair<const DirectX::XMFLOAT3*, UINT> Normals() const = 0;
	virtual std::pair<const DWORD*, UINT> Colors() const = 0;
	virtual std::pair<const UINT16*, UINT> Indices() const = 0;
};

class IFloorMesh : public IMesh {
public:
	static IFloorMesh* Create();
};