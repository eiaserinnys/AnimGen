#pragma once

#include <DX11Device.h>

#include "SceneDescriptor.h"

class DX11Render;
struct RenderContext;

class IObjectRenderer {
public:
	virtual ~IObjectRenderer();

	virtual void Render(const SceneDescriptor& sceneDesc) = 0;

	static IObjectRenderer* Create(RenderContext* context);
};