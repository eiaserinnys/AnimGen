#pragma once

#include <DX11Buffer.h>

//------------------------------------------------------------------------------
class LineBuffer {
public:
	LineBuffer(ID3D11Device* d3dDev);

	void EnsureBegin();

	void Begin();

	void End(ID3D11DeviceContext* devCtx);

	void EnqueueAnchor(const DirectX::XMFLOAT3& v, float length, DWORD c);

	void EnqueueFrame(const DirectX::XMMATRIX& m, float length);

	void Enqueue(std::vector<DirectX::XMFLOAT3>& p, DWORD c);

	void Draw(ID3D11DeviceContext* devCtx);

	//--------------------------------------------------------------------------
	std::unique_ptr<IDX11Buffer> posB;
	std::unique_ptr<IDX11Buffer> colB;
	std::unique_ptr<IDX11Buffer> indB;

	std::vector<DirectX::XMFLOAT3> pos;
	std::vector<DWORD> col;
	std::vector<UINT16> ind;

	bool needToBegin = true;
};
