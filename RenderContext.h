#pragma once

#include <memory>

#include <SpriteBatch.h>
#include <SpriteFont.h>

#include <DX11Device.h>
#include <ShaderManager.h>

struct RenderContext
{
	RenderContext(HWND hwnd);
	~RenderContext();

	void Reload();
	void ReloadShader();

	HWND hwnd;

	std::unique_ptr<DX11Device> d3d11;
	std::unique_ptr<IVertexShaderManager> vs;
	std::unique_ptr<IPixelShaderManager> ps;
};
