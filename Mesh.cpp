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
		float extent = 1;
		float height = 0;

		for (int i = -4; i <= 4; ++i)
		{
			for (int j = -4; j <= 4; ++j)
			{
				XMFLOAT3 center((float)i, 0, (float)j);

				AddRectangle(
					XMFLOAT3(center.x - extent / 2, height, center.z + extent / 2),
					XMFLOAT3(center.x + extent / 2, height, center.z + extent / 2),
					XMFLOAT3(center.x - extent / 2, height, center.z - extent / 2),
					XMFLOAT3(center.x + extent / 2, height, center.z - extent / 2),
					XMFLOAT3(0, 1, 0),
					(i + j) % 2 == 0 ? 0xff808080 : 0xff404040);
			}
		}
	}
};

//------------------------------------------------------------------------------
IFloorMesh* IFloorMesh::Create()
{ return new FloorMesh(); }