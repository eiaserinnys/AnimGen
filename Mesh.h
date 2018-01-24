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
	static IFloorMesh* Create(DWORD clr1, DWORD clr2);
};

class IBoxMesh : public IMesh {
public:
	static IBoxMesh* Create(
		const DirectX::XMFLOAT3& center, 
		const DirectX::XMFLOAT3& extent,
		DWORD clr);
};

class IArrowMesh : public IMesh {
public:
	static IArrowMesh* Create(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		float headLen,
		float radius1,
		float radius2,
		int granulity, 
		DWORD clr);
};

class ICoordinateAxisMesh : public IMesh {
public:
	static ICoordinateAxisMesh* Create(
		const DirectX::XMMATRIX& frame, 
		float length, 
		float headLen,
		float radius1,
		float radius2,
		int granulity);
};