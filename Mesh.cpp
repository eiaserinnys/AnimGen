#include "pch.h"

#include "Mesh.h"
#include "MeshT.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
IMesh::~IMesh()
{}

//------------------------------------------------------------------------------
class FloorMesh : public MeshT<IFloorMesh> {
public:
	FloorMesh()
	{
		float extent = 10;
		float height = 0;

		AddRectangle(
			XMFLOAT3(-extent / 2, +extent / 2, height), 
			XMFLOAT3(+extent / 2, +extent / 2, height), 
			XMFLOAT3(-extent / 2, -extent / 2, height), 
			XMFLOAT3(+extent / 2, -extent / 2, height),
			XMFLOAT3(0, 0, 1));
	}
};

//------------------------------------------------------------------------------
IFloorMesh* IFloorMesh::Create()
{ return new FloorMesh(); }