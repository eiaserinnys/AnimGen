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
	FloorMesh(DWORD clr1, DWORD clr2)
	{
		float extent = 1;
		float height = 0;

		for (int i = -8; i <= 8; ++i)
		{
			for (int j = -8; j <= 8; ++j)
			{
				XMFLOAT3 center((float)i, 0, (float)j);

				AddRectangle(
					center, 
					XMFLOAT3(extent / 2, 0, 0), 
					XMFLOAT3(0, 0, extent / 2),
					XMFLOAT3(0, 1, 0),
					(i + j) % 2 == 0 ? clr1 : clr2);
			}
		}
	}
};

//------------------------------------------------------------------------------
IFloorMesh* IFloorMesh::Create(DWORD clr1, DWORD clr2)
{ return new FloorMesh(clr1, clr2); }

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

