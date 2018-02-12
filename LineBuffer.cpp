#include "pch.h"
#include "LineBuffer.h"

#include <Utility.h>

#include "FrameHelper.h"

using namespace std;
using namespace DirectX;

LineBuffer::LineBuffer(ID3D11Device* d3dDev)
{
	int vbSize = 10000;
	int ibSize = 50000;

	posB.reset(IDX11Buffer::Create_DynamicVB(
		d3dDev, sizeof(DirectX::XMFLOAT3), vbSize * sizeof(DirectX::XMFLOAT3)));

	colB.reset(IDX11Buffer::Create_DynamicVB(
		d3dDev, sizeof(DWORD), vbSize * sizeof(DWORD)));

	indB.reset(IDX11Buffer::Create_DynamicIB(
		d3dDev, sizeof(UINT16), ibSize * sizeof(UINT16)));
}

//--------------------------------------------------------------------------
void LineBuffer::EnsureBegin()
{
	if (needToBegin) { Begin(); }
}

//--------------------------------------------------------------------------
void LineBuffer::Begin()
{
	needToBegin = false;
	pos.clear();
	col.clear();
	ind.clear();
}

//--------------------------------------------------------------------------
void LineBuffer::End(ID3D11DeviceContext* devCtx)
{
	if (!pos.empty())
	{
		posB->UpdateDiscard(devCtx, &pos[0], (UINT)pos.size());
		colB->UpdateDiscard(devCtx, &col[0], (UINT)col.size());
		indB->UpdateDiscard(devCtx, &ind[0], (UINT)ind.size());
	}

	needToBegin = true;
}

//--------------------------------------------------------------------------
void LineBuffer::EnqueueAnchor(const XMFLOAT3& v, float length, DWORD c)
{
	EnsureBegin();

	size_t pivot = pos.size();
	pos.push_back(v - XMFLOAT3(length / 2, 0, 0));
	pos.push_back(v + XMFLOAT3(length / 2, 0, 0));
	pos.push_back(v - XMFLOAT3(0, length / 2, 0));
	pos.push_back(v + XMFLOAT3(0, length / 2, 0));
	pos.push_back(v - XMFLOAT3(0, 0, length / 2));
	pos.push_back(v + XMFLOAT3(0, 0, length / 2));

	col.insert(col.end(), 6, c);

	ind.push_back(pivot++); ind.push_back(pivot++);
	ind.push_back(pivot++); ind.push_back(pivot++);
	ind.push_back(pivot++); ind.push_back(pivot++);
}

//--------------------------------------------------------------------------
void LineBuffer::EnqueueFrame(const XMMATRIX& m, float length)
{
	EnsureBegin();

	auto x = FrameHelper::GetX(m);
	auto y = FrameHelper::GetY(m);
	auto z = FrameHelper::GetZ(m);
	auto v = FrameHelper::GetTranslation(m);

	size_t pivot = pos.size();
	pos.push_back(v); pos.push_back(v + x * length);
	pos.push_back(v); pos.push_back(v + y * length);
	pos.push_back(v); pos.push_back(v + z * length);

	col.push_back(0xff0000ff); col.push_back(0xff0000ff);
	col.push_back(0xff00ff00); col.push_back(0xff00ff00);
	col.push_back(0xffff0000); col.push_back(0xffff0000);

	ind.push_back(pivot++); ind.push_back(pivot++);
	ind.push_back(pivot++); ind.push_back(pivot++);
	ind.push_back(pivot++); ind.push_back(pivot++);
}

//--------------------------------------------------------------------------
void LineBuffer::Enqueue(vector<XMFLOAT3>& p, DWORD c)
{
	EnsureBegin();

	size_t pivot = pos.size();
	pos.reserve(pos.size() + p.size());
	pos.insert(pos.end(), p.begin(), p.end());

	col.reserve(col.size() + p.size());
	col.insert(col.end(), p.size(), c);

	for (size_t i = 0; i + 1 < p.size(); ++i)
	{
		ind.push_back(i + pivot);
		ind.push_back(i + 1 + pivot);
	}
}

//--------------------------------------------------------------------------
void LineBuffer::Draw(ID3D11DeviceContext* devCtx)
{
	posB->ApplyVB(devCtx, 0, 0);
	colB->ApplyVB(devCtx, 1, 0);
	indB->ApplyIB(devCtx, 0);

	devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	devCtx->DrawIndexed((UINT)ind.size(), 0, 0);
}
