#pragma once

#include "SceneDescriptor.h"

struct RenderContext;
class LineBuffer;

class IDiagnosticRenderer {
public:
	virtual ~IDiagnosticRenderer();

	virtual void Render(const SceneDescriptor& sceneDesc) = 0;

	virtual LineBuffer* Buffer() = 0;

	static IDiagnosticRenderer* Create(RenderContext* context);
};