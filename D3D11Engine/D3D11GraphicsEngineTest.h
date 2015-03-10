#pragma once
#include "d3d11graphicsengine.h"
class D3D11GraphicsEngineTest :
	public D3D11GraphicsEngine
{
public:
	D3D11GraphicsEngineTest(void);
	~D3D11GraphicsEngineTest(void);

	/** Draws the worldmesh */
	void DrawWorldMeshTest();
	
	/** Draws vobs */
	void DrawVobsTest();

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering();
};

