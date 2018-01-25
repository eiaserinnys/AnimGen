#include "pch.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "Mesh.h"
#include "MeshT.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
IMesh::~IMesh()
{}

//------------------------------------------------------------------------------
IUIMesh::~IUIMesh()
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

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
class ArrowMesh : public MeshT<IArrowMesh> {
public:
	ArrowMesh(
		const XMFLOAT3& start,
		const XMFLOAT3& end,
		float headLen,
		float radius1,
		float radius2,
		int granulity,
		DWORD clr)
	{
		// radius1이 작게 한다
		if (radius2 < radius1) { swap(radius1, radius2); }

		XMFLOAT3 dir = end - start;
		float length = Length(dir);
		dir = Normalize(dir);

		if (length <= 0) { throw invalid_argument("화살의 길이가 0일 수는 없습니다."); }

		if (headLen > length * 0.5f) { headLen = length * 0.5f; }

		float bodyLen = length - headLen;

		// 화살 방향과 직교하는 축을 구성한다
		XMFLOAT3 refDir = XMFLOAT3(0, 0, 1);
		if (Dot(refDir, dir) > 0.95)
		{
			refDir = XMFLOAT3(0, 1, 0);
		}

		XMFLOAT3 x = Normalize(Cross(refDir, dir));
		XMFLOAT3 y = Normalize(Cross(dir, x));

		// 작은 쪽 캡을 만든다
		{
			UINT16 centerInd = pos.size();
			pos.push_back(start);
			nor.push_back(-dir);
			col.push_back(clr);

			UINT16 pivotInd = pos.size();
			for (int i = 0; i < granulity; ++i)
			{
				float angle = ((float)i / granulity) * M_PI * 2;
				XMFLOAT3 p = start + x * cosf(angle) * radius1 + y * sinf(angle) * radius1;
				pos.push_back(p);
				nor.push_back(-dir);
				col.push_back(clr);

				ind.push_back(centerInd);
				ind.push_back(pivotInd + i);
				ind.push_back(pivotInd + (i + 1) % granulity);
			}
		}

		// 실린더를 만든다
		{
			UINT16 pivotInd = pos.size();
			for (int i = 0; i < granulity; ++i)
			{
				float angle = ((float)i / granulity) * M_PI * 2;
				XMFLOAT3 p = start + x * cosf(angle) * radius1 + y * sinf(angle) * radius1;
				pos.push_back(p);
				nor.push_back(x * cosf(angle) + y * sinf(angle));
				col.push_back(clr);

				XMFLOAT3 q = start + dir * bodyLen + x * cosf(angle) * radius1 + y * sinf(angle) * radius1;
				pos.push_back(q);
				nor.push_back(x * cosf(angle) + y * sinf(angle));
				col.push_back(clr);

				int j = (i + 1) % granulity;
				ind.push_back(pivotInd + i * 2 + 0);
				ind.push_back(pivotInd + i * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 0);

				ind.push_back(pivotInd + i * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 0);
			}
		}
		
		// 헤드 쪽 캡을 만든다
		{
			UINT16 pivotInd = pos.size();
			for (int i = 0; i < granulity; ++i)
			{
				float angle = ((float)i / granulity) * M_PI * 2;
				XMFLOAT3 p = start + dir * bodyLen + x * cosf(angle) * radius1 + y * sinf(angle) * radius1;
				pos.push_back(p);
				nor.push_back(-dir);
				col.push_back(clr);

				XMFLOAT3 q = start + dir * bodyLen + x * cosf(angle) * radius2 + y * sinf(angle) * radius2;
				pos.push_back(q);
				nor.push_back(-dir);
				col.push_back(clr);

				int j = (i + 1) % granulity;
				ind.push_back(pivotInd + i * 2 + 0);
				ind.push_back(pivotInd + i * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 0);

				ind.push_back(pivotInd + i * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 1);
				ind.push_back(pivotInd + j * 2 + 0);
			}
		}

		// 헤드를 닫는다
		{
			UINT16 centerInd = pos.size();
			pos.push_back(end);
			nor.push_back(dir);
			col.push_back(clr);

			UINT16 pivotInd = pos.size();
			for (int i = 0; i < granulity; ++i)
			{
				float angle = ((float)i / granulity) * M_PI * 2;
				XMFLOAT3 p = start + dir * bodyLen + x * cosf(angle) * radius2 + y * sinf(angle) * radius2;
				pos.push_back(p);
				nor.push_back(x * cosf(angle) + y * sinf(angle));
				col.push_back(clr);

				ind.push_back(centerInd);
				ind.push_back(pivotInd + (i + 1) % granulity);
				ind.push_back(pivotInd + i);
			}
		}
	}
};

//------------------------------------------------------------------------------
IArrowMesh* IArrowMesh::Create(
	const XMFLOAT3& start, 
	const XMFLOAT3& end, 
	float headLen,
	float radius1,
	float radius2,
	int granulity,
	DWORD clr)
{
	return new ArrowMesh(start, end, headLen, radius1, radius2, granulity, clr);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
class CoordinateAxisMesh : public MeshT<ICoordinateAxisMesh> {
public:
	CoordinateAxisMesh(
		const XMMATRIX& frame,
		float length,
		float headLen,
		float radius1,
		float radius2,
		int granulity)
	{
		XMFLOAT3 xAxis(
			frame.r[0].m128_f32[0], 
			frame.r[0].m128_f32[1], 
			frame.r[0].m128_f32[2]);
		XMFLOAT3 yAxis(
			frame.r[1].m128_f32[0], 
			frame.r[1].m128_f32[1], 
			frame.r[1].m128_f32[2]);
		XMFLOAT3 zAxis(
			frame.r[2].m128_f32[0], 
			frame.r[2].m128_f32[1], 
			frame.r[2].m128_f32[2]);

		xAxis = Normalize(xAxis);
		yAxis = Normalize(yAxis);
		zAxis = Normalize(zAxis);

		XMFLOAT3 org(
			frame.r[3].m128_f32[0],
			frame.r[3].m128_f32[1],
			frame.r[3].m128_f32[2]);

		unique_ptr<IArrowMesh> x(IArrowMesh::Create(
			org, org + xAxis * length, headLen, radius1, radius2, granulity, 0x640000ff));
		unique_ptr<IArrowMesh> y(IArrowMesh::Create(
			org, org + yAxis * length, headLen, radius1, radius2, granulity, 0x6400ff00));
		unique_ptr<IArrowMesh> z(IArrowMesh::Create(
			org, org + zAxis * length, headLen, radius1, radius2, granulity, 0x64ff0000));

		Append(x.get());
		Append(y.get());
		Append(z.get());
	}
};

ICoordinateAxisMesh* ICoordinateAxisMesh::Create(
	const XMMATRIX& frame,
	float length,
	float headLen,
	float radius1,
	float radius2,
	int granulity)
{
	return new CoordinateAxisMesh(frame, length, headLen, radius1, radius2, granulity);
}