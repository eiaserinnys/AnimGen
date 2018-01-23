#pragma once

#include "SceneDescriptor.h"

struct RenderContext;

class IDeferredRenderer {
public:
	virtual ~IDeferredRenderer();

	virtual void Render(const SceneDescriptor& sceneDesc) = 0;

	static IDeferredRenderer* Create(RenderContext* context);
};