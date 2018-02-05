#include "pch.h"
#include "FrameHelper.h"

#include "Vector.h"

using namespace Core;
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
static Vector3D GetAxis(const Matrix4D& tx)
{
	return Vector3D(
		tx.m[index][0],
		tx.m[index][1],
		tx.m[index][2]);
}

Vector3D FrameHelper::GetX(const Matrix4D& tx)
{ return GetAxis<0>(tx); }

Vector3D FrameHelper::GetY(const Matrix4D& tx)
{ return GetAxis<1>(tx); }

Vector3D FrameHelper::GetZ(const Matrix4D& tx)
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

template <int index>
static void SetAxis(Matrix4D& tx, const Vector3D& v)
{
	tx.m[index][0] = v.x;
	tx.m[index][1] = v.y;
	tx.m[index][2] = v.z;
}

void FrameHelper::SetX(Matrix4D& tx, const Vector3D& v)
{ SetAxis<0>(tx, v); }

void FrameHelper::SetY(Matrix4D& tx, const Vector3D& v)
{ SetAxis<1>(tx, v); }

void FrameHelper::SetZ(Matrix4D& tx, const Vector3D& v)
{ SetAxis<2>(tx, v); }

void FrameHelper::Set(
	XMMATRIX& tx, 
	const XMFLOAT3& x, 
	const XMFLOAT3& y,
	const XMFLOAT3& z)
{
	SetAxis<0>(tx, x);
	SetAxis<1>(tx, y);
	SetAxis<2>(tx, z);
}

void FrameHelper::Set(
	Matrix4D& tx, 
	const Vector3D& x, 
	const Vector3D& y,
	const Vector3D& z)
{
	SetAxis<0>(tx, x);
	SetAxis<1>(tx, y);
	SetAxis<2>(tx, z);
}

void FrameHelper::SetTranslation(XMMATRIX& tx, const XMFLOAT3& pos)
{
	tx.r[3].m128_f32[0] = pos.x;
	tx.r[3].m128_f32[1] = pos.y;
	tx.r[3].m128_f32[2] = pos.z;
}

XMFLOAT3 FrameHelper::GetTranslation(const XMMATRIX& tx)
{
	return XMFLOAT3(tx.r[3].m128_f32[0], tx.r[3].m128_f32[1], tx.r[3].m128_f32[2]);
}

void FrameHelper::SetTranslation(Matrix4D& tx, const Vector3D& pos)
{
	tx.m[3][0] = pos.x;
	tx.m[3][1] = pos.y;
	tx.m[3][2] = pos.z;
}

Vector3D FrameHelper::GetTranslation(const Matrix4D& tx)
{
	return Vector3D(tx.m[3][0], tx.m[3][1], tx.m[3][2]);
}
