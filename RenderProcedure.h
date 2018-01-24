#pragma once

#undef new
#undef delete

#include <vector>
#include <list>

#include <DirectXMath.h>
#include <DX11Buffer.h>

#include "SceneDescriptor.h"

class IToRender
{
public:
	virtual ~IToRender();
	virtual void Render() = 0;
};

struct RenderFlag
{
	enum Value
	{
		VertexColor,
		VertexColorLit,
		Textured,
		Deformed,
	};
};

struct Wireframe
{
	enum Value
	{
		True,
		False,
		Argument,
	};
};

struct BakeFlag
{
	enum Value
	{
		None,
		Unwrap,
		DepthUpsample,
		WLS,
	};
};

struct RenderTuple
{
	IToRender* render = nullptr;
	Wireframe::Value wireframe = Wireframe::False;
	bool noZCompare = false;
	RenderFlag::Value flag = RenderFlag::VertexColor;
	DirectX::XMFLOAT4X4 world;
	std::string texture;
	bool lit = true;

	DirectX::XMMATRIX* nodeTx = nullptr;
	DirectX::XMFLOAT3* nodePos = nullptr;
	int nodeCount = 0;
};

class RenderProcedure {
public:
	RenderProcedure(HWND hwnd, DX11Device* device, IRenderTargetManager* rts);

	void Begin();
	void End();

	void Bake(IToRender* render, const std::string& srcTexture, const std::wstring& bakeFileName);

	void* operator new(std::size_t size) { return _aligned_malloc(size, 16); }
	void operator delete(void* ptr) { return _aligned_free(ptr); }

private:
	DX11Device* device = nullptr;
	IRenderTargetManager* rts = nullptr;
	HWND hwnd;
};