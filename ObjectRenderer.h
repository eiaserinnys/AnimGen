#pragma once

#include "SceneDescriptor.h"

struct RenderContext;

class IObjectRenderer {
public:
	virtual ~IObjectRenderer();

	virtual void FillBuffer() = 0;
	virtual void Render(const SceneDescriptor& sceneDesc) = 0;

	static IObjectRenderer* Create(RenderContext* context);
};