#include "pch.h"
#include "IKPicking.h"

#include "VectorDXMathAdaptor.h"
#include "RenderContext.h"
#include "Robot.h"

using namespace std;
using namespace Core;
using namespace DirectX;

class IKPicking : public IIKPicking {
public:
	void Update(
		IRobot* robot, 
		const SceneDescriptor& sceneDesc, 
		RenderContext* context)
	{
		posW[0] = ToXMFLOAT3(robot->GetWorldPosition("LFoot"));
		posW[1] = ToXMFLOAT3(robot->GetWorldPosition("RFoot"));

		posN[0] = sceneDesc.GetNdc(posW[0]);
		posN[1] = sceneDesc.GetNdc(posW[1]);

		posS[0] = sceneDesc.GetScreenCoordinate(posW[0]);
		posS[1] = sceneDesc.GetScreenCoordinate(posW[1]);

		XMFLOAT2 dragPos(
			scrPos.x + mousePos.x - mouseDownPos.x,
			scrPos.y + mousePos.y - mouseDownPos.y);

		for (int i = 0; i < COUNT_OF(posS); ++i)
		{
			bool drag = dragIndex == i;

			context->uiRenderer->Enqueue(
				drag ? dragPos : XMFLOAT2(posS[i].x, posS[i].y),
				radius,
				0.01f,
				drag ?
					0xd0ffffff :
					(mouseOn[i] ? 0x60ffffff : 0x20ffffff));

			if (drag)
			{
				auto posW = sceneDesc.GetWorldPositionByScreenCoordinate(dragPos, posN[i].w);

				robot->SetFootPosition(i == 0, FromXMFLOAT3(posW));
			}
		}

		scrValid = true;
	}

	XMFLOAT3 posW[2];
	XMFLOAT4 posN[2];
	XMFLOAT3 posS[2];
	
	bool mouseOn[2] = { false, false };
	bool scrValid = false;
	const float radius = 30;

	int dragIndex = -1;
	XMFLOAT3 mouseDownPos;
	XMFLOAT3 mousePos;
	XMFLOAT3 scrPos;

	bool IsDragging() const { return dragIndex >= 0; }

	bool BeginDrag(HWND hWnd, const XMFLOAT3& mouseDownPos_)
	{
		if (mouseOn[0]) { dragIndex = 0; }
		else if (mouseOn[1]) { dragIndex = 1; }

		if (IsDragging())
		{
			SetCapture(hWnd);
			mouseDownPos = mouseDownPos_;
			mousePos = mouseDownPos_;
			scrPos = posS[dragIndex];
			return true;
		}
		return false;
	}

	bool EndDrag()
	{
		if (IsDragging())
		{
			ReleaseCapture();
			dragIndex = -1;
			return true;
		}
		return false;
	}

	LRESULT HandleMessages(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		// Current mouse position
		int mouseX = (short)LOWORD(lParam);
		int mouseY = (short)HIWORD(lParam);

		if (scrValid)
		{
			switch (uMsg) {
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
				return BeginDrag(hWnd, XMFLOAT3(mouseX, mouseY, 0));

			case WM_LBUTTONUP:
				return EndDrag();

			case WM_CAPTURECHANGED:
				if ((HWND)lParam != hWnd) { return EndDrag(); }
				return FALSE;

			case WM_MOUSEMOVE:
				if (!IsDragging())
				{
					mouseOn[0] = false;
					mouseOn[1] = false;

					XMFLOAT2 dist[] = {
						{ mouseX - posS[0].x, mouseY - posS[0].y },
						{ mouseX - posS[1].x, mouseY - posS[1].y },
					};
					bool on[] = 
					{
						Length(dist[0]) < radius, 
						Length(dist[1]) < radius,
					};

					if (posS[0].x < posS[1].x)
					{
						mouseOn[0] = on[0];
						if (!mouseOn[0]) { mouseOn[1] = on[1]; }
					}
					else
					{
						mouseOn[1] = on[1];
						if (!mouseOn[1]) { mouseOn[0] = on[0]; }
					}
				}
				else
				{
					mousePos = XMFLOAT3(mouseX, mouseY, 0);
				}
				return FALSE;
			}
		}

		return FALSE;
	}
};

IIKPicking::~IIKPicking()
{}

IIKPicking* IIKPicking::Create()
{ return new IKPicking; }