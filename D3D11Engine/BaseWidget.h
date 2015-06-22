#pragma once
#include "pch.h"

const float BASEWIDGET_TRANS_LENGTH = 1.5;
const float BASEWIDGET_CONE_LENGTH = 1.25;
const float BASEWIDGET_CONE_RADIUS = 0.125;
const float BASEWIDGET_CUBE_EXTENDS = 0.125;

const int BASEWIDGET_MODE_TRANS_ROT = 0;
const int BASEWIDGET_MODE_SCALE = 1;

class EditorLinePrimitive;
class WidgetContainer;
class zCVob;
class BaseWidget
{
public:
	BaseWidget(WidgetContainer* container);
	virtual ~BaseWidget(void);

	/** Renders the widget */
	virtual void RenderWidget();

	/** Called when a mousebutton was clicked */
	virtual void OnMButtonClick(int button);

	/** Called when the owning window got a message */
	virtual void OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** Called when an object was added to the selection */
	virtual void OnSelectionAdded(zCVob* vob);

	/** Widget primitives */
	void CreateArrowCone(int Detail, int Axis, D3DXVECTOR4* Color, EditorLinePrimitive* Prim);
	void CreateArrowCube(D3DXVECTOR3* Offset, float Extends, D3DXVECTOR4* Color, EditorLinePrimitive* Prim);

	/** Returns whether this widget is active or not */
	virtual bool IsActive(){return false;}

protected:
	/** Captures the mouse in the middle of the screen and returns the delta since last frame */
	D3DXVECTOR2 GetMouseDelta();

	/** Hides/Shows the mouse */
	void SetMouseVisibility(bool visible);

	/** Transforms of the widget */
	D3DXVECTOR3 Position;
	D3DXMATRIX Rotation;
	D3DXVECTOR3 Scale;

	/** Owning widgetcontainer */
	WidgetContainer* OwningContainer;

	EditorLinePrimitive* testPrim;
};

