#pragma once

#include <memory>

#include <DX11Device.h>
#include <RenderTargetManager.h>
#include <ShaderDefineManager.h>
#include <TextureManager.h>
#include <TextRenderer.h>

#include "Mesh.h"
#include "ObjectBuffer.h"

#include "UIRenderer.h"
#include "ObjectRenderer.h"
#include "DeferredRenderer.h"

#include "Robot.h"

#include "Logger.h"

struct RenderContext
{
	RenderContext(HWND hwnd);
	~RenderContext();

	void Reload();

	void FillBuffer();

	HWND hwnd;

	std::unique_ptr<DX11Device> d3d11;
	std::unique_ptr<IRenderTargetManager> rts;
	std::unique_ptr<IShaderDefineManager> sd;
	std::unique_ptr<ITextureManager> textures;
	
	std::unique_ptr<IObjectRenderer> objRenderer;
	std::unique_ptr<IDeferredRenderer> deferredRenderer;
	std::unique_ptr<ITextRenderer> textRenderer;
	std::unique_ptr<IUIRenderer> uiRenderer;

	std::unique_ptr<ObjectBuffer> objBuffer;
	std::unique_ptr<IMesh> floor;
	std::vector<IMesh*> etc;

	std::unique_ptr<IRobot> robot;

	std::unique_ptr<Logger> logger;
};
