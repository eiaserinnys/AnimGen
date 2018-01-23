#pragma once

#include <memory>

#include <DX11Device.h>
#include <RenderTargetManager.h>
#include <ShaderDefineManager.h>

#include "Mesh.h"
#include "ObjectBuffer.h"

#include "ObjectRenderer.h"
#include "DeferredRenderer.h"

struct RenderContext
{
	RenderContext(HWND hwnd);
	~RenderContext();

	void Reload();
	void ReloadShader();

	void FillBuffer();

	HWND hwnd;

	std::unique_ptr<DX11Device> d3d11;
	std::unique_ptr<IRenderTargetManager> rts;
	std::unique_ptr<IShaderDefineManager> sd;
	
	std::unique_ptr<IObjectRenderer> objRenderer;
	std::unique_ptr<IDeferredRenderer> deferredRenderer;

	std::unique_ptr<ObjectBuffer> objBuffer;
	std::unique_ptr<IMesh> floor;
	std::unique_ptr<IMesh> box;
};
