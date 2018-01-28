#include "pch.h"
#include "FrameHelper.h"

using namespace DirectX;

template <int index>
static XMFLOAT3 GetAxis(const XMMATRIX& tx)
{
	return XMFLOAT3(
		tx.r[index].m128_f32[0],
		tx.r[index].m128_f32[1],
		tx.r[index].m128_f32[2]);
}

XMFLOAT3 FrameHelper::GetX(const XMMATRIX& tx)
{ return GetAxis<0>(tx); }

XMFLOAT3 FrameHelper::GetY(const XMMATRIX& tx)
{ return GetAxis<1>(tx); }

XMFLOAT3 FrameHelper::GetZ(const XMMATRIX& tx)
{ return GetAxis<2>(tx); }

template <int index>
static void SetAxis(XMMATRIX& tx, const XMFLOAT3& v)
{
	tx.r[index].m128_f32[0] = v.x;
	tx.r[index].m128_f32[1] = v.y;
	tx.r[index].m128_f32[2] = v.z;
}

void FrameHelper::SetX(XMMATRIX& tx, const XMFLOAT3& v)
{ SetAxis<0>(tx, v); }

void FrameHelper::SetY(XMMATRIX& tx, const XMFLOAT3& v)
{ SetAxis<1>(tx, v); }

void FrameHelper::SetZ(XMMATRIX& tx, const XMFLOAT3& v)
{ SetAxis<2>(tx, v); }

