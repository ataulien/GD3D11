#pragma once
#include "d2dsubview.h"
#include "WorldConverter.h"

struct RenderToTextureBuffer;
struct RenderToDepthStencilBuffer;
class SV_Panel;
class SV_GMeshInfoView :
	public D2DSubView
{
public:
	SV_GMeshInfoView(D2DView* view, D2DSubView* parent);
	~SV_GMeshInfoView(void);

	enum ERenderMode
	{
		RM_Wireframe = 0,
		RM_TexturedWireFrame = 1,
		RM_Textured = 2,
		RM_Lit = 3
	};

	/** Sets the current render mode */
	void SetRenderMode(ERenderMode mode);

	/** Sets the mesh infos for this view */
	void SetMeshes(const std::map<zCTexture*, MeshInfo*>& meshes, BaseVisualInfo* visInfo = NULL);

	/** Sets the name of this view */
	void SetName(const std::string& name);

	/** Sets the rotation of this object in the view */
	void SetObjectOrientation(float yaw, float pitch, float distance);

	/** Updates the view */
	void UpdateView();

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

	/** Processes a window-message. Return false to stop the message from going to children */
	bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Returns the view-properties */
	void GetObjectOrientation(float& yaw, float& pitch, float& distance);

protected:
	/** Draws the meshes to the buffer */
	void DrawMeshes();

	std::map<zCTexture*, MeshInfo*> Meshes;
	BaseVisualInfo* VisualInfo;
	ERenderMode RenderMode;

	/** Orientation */
	float ObjectYaw;
	float ObjectPitch;
	float ObjectDistance;
	D3DXVECTOR3 ObjectPosition;
	float FOV;
	D3DXMATRIX ObjectWorldMatrix;
	D3DXMATRIX ObjectViewMatrix;
	D3DXMATRIX ObjectProjMatrix;

	/** Props */
	bool IsDraggingView;
	POINT LastDragPosition;

	/** Rendertarget */
	RenderToTextureBuffer* RT;
	RenderToDepthStencilBuffer* DS;

	/** Image panel */
	SV_Panel* Panel;
};

