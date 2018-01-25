#pragma once

#include "SceneDescriptor.h"

struct RenderContext;

class IUIRenderer {
public:
	virtual ~IUIRenderer();

	virtual void Render(const DirectX::XMFLOAT2& extent) = 0;

	static IUIRenderer* Create(RenderContext* context);
};