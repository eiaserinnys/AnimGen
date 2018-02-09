#pragma once

#undef new
#undef delete

#include <vector>
#include <list>

#include <DirectXMath.h>
#include <DX11Buffer.h>

#include "SceneDescriptor.h"
#include "RenderContext.h"

struct Wireframe
{
	enum Value
	{
		True,
		False,
		Argument,
	};
};

class IRenderProcedure {
public:
	virtual ~IRenderProcedure() {}

	static IRenderProcedure* Create(RenderContext* context);

	virtual void Render(const SceneDescriptor& sceneDesc) = 0;
};