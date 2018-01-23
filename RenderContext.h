#pragma once

#include <memory>

#include <DX11Device.h>
#include <RenderTargetManager.h>
#include <ShaderManager.h>

#include "ObjectRenderer.h"
#include "DeferredRenderer.h"

struct RenderContext
{
	RenderContext(HWND hwnd);
	~RenderContext();

	void Reload();
	void ReloadShader();

	HWND hwnd;

	std::unique_ptr<DX11Device> d3d11;
	std::unique_ptr<IRenderTargetManager> rts;
	std::unique_ptr<IVertexShaderManager> vs;
	std::unique_ptr<IPixelShaderManager> ps;
	
	std::unique_ptr<IObjectRenderer> objRenderer;
	std::unique_ptr<IDeferredRenderer> deferredRenderer;
};
