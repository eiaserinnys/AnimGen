#pragma once

class IMesh {
public:
	virtual ~IMesh();

	virtual std::pair<const DirectX::XMFLOAT3*, std::size_t> Vertices() const = 0;
	virtual std::pair<const DirectX::XMFLOAT3*, std::size_t> Normals() const = 0;
	virtual std::pair<const UINT16*, std::size_t> Indices() const = 0;
};

class IFloorMesh : public IMesh {
public:
	static IFloorMesh* Create();
};