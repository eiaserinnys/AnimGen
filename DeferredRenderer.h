#pragma once

struct RenderContext;

class IDeferredRenderer {
public:
	virtual ~IDeferredRenderer();

	virtual void Render() = 0;

	static IDeferredRenderer* Create(RenderContext* context);
};