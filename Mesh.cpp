#include "pch.h"

#include "Mesh.h"
#include "MeshT.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
IMesh::~IMesh()
{}

////////////////////////////////////////////////////////////////////////////////

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

				//AddRectangle(
				//	XMFLOAT3(center.x - extent / 2, height, center.z + extent / 2),
				//	XMFLOAT3(center.x + extent / 2, height, center.z + extent / 2),
				//	XMFLOAT3(center.x - extent / 2, height, center.z - extent / 2),
				//	XMFLOAT3(center.x + extent / 2, height, center.z - extent / 2),
				//	XMFLOAT3(0, 1, 0),
				//	(i + j) % 2 == 0 ? 0xff808080 : 0xff404040);

				AddRectangle(
					center, 
					XMFLOAT3(extent / 2, 0, 0), 
					XMFLOAT3(0, 0, extent / 2),
					XMFLOAT3(0, 1, 0),
					(i + j) % 2 == 0 ? 0xff808080 : 0xff404040);
			}
		}
	}
};

//------------------------------------------------------------------------------
IFloorMesh* IFloorMesh::Create()
{ return new FloorMesh(); }

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
class BoxMesh : public MeshT<IBoxMesh> {
public:
	BoxMesh(const XMFLOAT3& center, const XMFLOAT3& extent, DWORD clr)
	{
		{
			XMFLOAT3 c(center.x + extent.x / 2, center.y, center.z);

			AddRectangle(
				c, 
				XMFLOAT3(0, 0, extent.z / 2),
				XMFLOAT3(0, extent.y / 2, 0),
				XMFLOAT3(1, 0, 0),
				clr);
		}

		{
			XMFLOAT3 c(center.x - extent.x / 2, center.y, center.z);

			AddRectangle(
				c,
				XMFLOAT3(0, 0, -extent.z / 2),
				XMFLOAT3(0, extent.y / 2, 0),
				XMFLOAT3(- 1, 0, 0),
				clr);
		}

		{
			XMFLOAT3 c(center.x, center.y, center.z + extent.z / 2);

			AddRectangle(
				c,
				XMFLOAT3(-extent.x / 2, 0, 0),
				XMFLOAT3(0, extent.y / 2, 0),
				XMFLOAT3(0, 0, 1),
				clr);
		}

		{
			XMFLOAT3 c(center.x, center.y, center.z - extent.z / 2);

			AddRectangle(
				c,
				XMFLOAT3(extent.x / 2, 0, 0),
				XMFLOAT3(0, extent.y / 2, 0),
				XMFLOAT3(0, 0, -1),
				clr);
		}

		{
			XMFLOAT3 c(center.x, center.y + extent.y / 2, center.z);

			AddRectangle(
				c,
				XMFLOAT3(extent.x / 2, 0, 0),
				XMFLOAT3(0, 0, extent.z / 2),
				XMFLOAT3(0, 1, 0),
				clr);
		}

		{
			XMFLOAT3 c(center.x, center.y - extent.y / 2, center.z);

			AddRectangle(
				c,
				XMFLOAT3(-extent.x / 2, 0, 0),
				XMFLOAT3(0, 0, extent.z / 2),
				XMFLOAT3(0, -1, 0),
				clr);
		}
	}
};

//------------------------------------------------------------------------------
IBoxMesh* IBoxMesh::Create(const XMFLOAT3& center, const XMFLOAT3& extent, DWORD clr)
{
	return new BoxMesh(center, extent, clr);
}

