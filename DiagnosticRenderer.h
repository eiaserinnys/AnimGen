#pragma once

#include "SceneDescriptor.h"

struct RenderContext;

class IDiagnosticRenderer {
public:
	virtual ~IDiagnosticRenderer();

	virtual void Render(const SceneDescriptor& sceneDesc) = 0;

	static IDiagnosticRenderer* Create(RenderContext* context);
};