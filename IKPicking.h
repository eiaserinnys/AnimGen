#pragma once

struct RenderContext;
struct SceneDescriptor;
class IRobot;

class IIKPicking {
public:
	virtual ~IIKPicking();
	
	virtual void Update(
		IRobot* robot, 
		const SceneDescriptor& sceneDesc,
		RenderContext* context) = 0;

	virtual LRESULT HandleMessages(
		_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

	static IIKPicking* Create();
};