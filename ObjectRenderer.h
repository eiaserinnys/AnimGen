#pragma once

#include "SceneDescriptor.h"

struct RenderContext;
struct ObjectBuffer;

class IObjectRenderer {
public:
	virtual ~IObjectRenderer();

	virtual void Render(
		const SceneDescriptor& sceneDesc,
		ObjectBuffer* objBuffer) = 0;

	virtual void RenderShadow(
		const SceneDescriptor& sceneDesc,
		ObjectBuffer* objBuffer) = 0;

	static IObjectRenderer* Create(RenderContext* context);
};