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

class RenderProcedure {
public:
	RenderProcedure(RenderContext* context);

	void Render(const SceneDescriptor& sceneDesc);

	void Begin();
	void End();

private:
	RenderContext* context;
};