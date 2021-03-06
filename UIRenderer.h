#pragma once

#include "SceneDescriptor.h"

struct RenderContext;
struct SceneDescriptor;

class IUIRenderer {
public:
	virtual ~IUIRenderer();

	virtual void Enqueue(
		const DirectX::XMFLOAT2& pos, 
		float radiusInPixel, 
		float zDepth,
		DWORD color) = 0;

	virtual void Render(
		const SceneDescriptor& sceneDesc, 
		const DirectX::XMFLOAT2& extent) = 0;

	static IUIRenderer* Create(RenderContext* context);
};