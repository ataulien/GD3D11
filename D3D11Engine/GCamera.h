#pragma once
#include "pch.h"

class GCamera
{
public:
	GCamera(void);
	~GCamera(void);



	/** Recreates the view matrix. Only call after you changed a value! */
	void RecreateViewMatrix(){};

	/** Returns the View-Matrix*/
	D3DXMATRIX& GetView(){return View;}

	/** Returns the WorldPosition */
	D3DXVECTOR3& GetWorldPosition(){return WorldPosition;}

	/** Returns the WorldPosition */
	D3DXVECTOR3& GetLookAt(){return LookAt;}

protected:
	/** Cameras view matrix */
	D3DXMATRIX View;

	/** Cameras position */
	D3DXVECTOR3 WorldPosition;

	/** Cameras Look-At position */
	D3DXVECTOR3 LookAt;
};

