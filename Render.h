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

struct TextToRender
{
	TextToRender(
		const DirectX::XMFLOAT3& p,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: pos(p), ofs(0, 0), is3d(true), text(t), clr(c) {}

	TextToRender(
		const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT2& o,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: pos(p), ofs(o), is3d(true), text(t), clr(c) {}

	TextToRender(
		const DirectX::XMFLOAT2& p,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: ofs(p), is3d(false), text(t), clr(c) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 ofs;
	bool is3d;

	std::wstring text;
	DirectX::XMFLOAT4 clr;
};

struct DX11Render
{
	DX11Render(HWND hwnd, DX11Device* device);

	void Begin(BakeFlag::Value bake = BakeFlag::None);
	void Render(RenderTuple* tuple, int tupleCount, bool wireframe);
	void End();

	void Bake(IToRender* render, const std::string& srcTexture, const std::wstring& bakeFileName);

	void UpsampleDepth(IToRender* render, std::vector<float>& buffer, bool fhd);

	void* operator new(std::size_t size) { return _aligned_malloc(size, 16); }
	void operator delete(void* ptr) { return _aligned_free(ptr); }

	void RenderText(const TextToRender& text) { textToRender.push_back(text); }
	void RenderText(const DirectX::XMMATRIX& wvp);

private:

public:
	DX11Device* device = nullptr;
	HWND hwnd;
	std::list<TextToRender> textToRender;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> font;
};