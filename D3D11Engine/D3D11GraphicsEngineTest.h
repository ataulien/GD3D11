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

	/** Collects the renderlist for the world-mesh */
	void GetWorldMeshRenderList(std::list<std::pair<MeshKey, MeshInfo *>>& list);

	/** Draws a world mesh render list for the given stage */
	void DrawWorldMeshRenderList(int stage, const std::list<std::pair<MeshKey, MeshInfo *>>& list);

	/** Draws the scene using the light-pre-pass technique. */
	void DrawSceneLightPrePass();

	/** Fills the Instancing-Buffer with the collected visible vobs.
		Also sets the start index in the visuals for this frame. */
	XRESULT FillInstancingBuffer(const std::vector<VobInfo *>& vobs);

	/** Marks all given vobs as not visible in the current frame */
	void MarkVobNonVisibleInFrame(const std::vector<VobInfo *>& vobs);

	/** Binds the distance-buffer for the given visual.
		Needed for smooth fade-out in the distance.
		 - visual: The visual to bind the buffer for. If NULL, bind FLT_MAX.
		 - slot: Pixelshader-Slot to bind to */
	void BindDistanceInformationFor(MeshVisualInfo* visual, int slot = 3);

	/** Binds the shader and the textures for the given mesh-key.
		 - Returns false if not cached in */
	bool BindShaderForKey(const MeshKey& key);

	/** Binds the PNAEN-Tesselation shaders if possible. Removes them from the pipeline otherwise */
	bool TryBindPNAENShaderForVisual(MeshVisualInfo* visual, MeshInfo* mesh);

	/** Draws a StaticMeshVisual instanced */
	void DrawVisualInstances(MeshVisualInfo* visual);

	/** Binds the right shader for the given texture */
	void BindShaderForTexturePrePass(zCTexture* texture, bool forceAlphaTest=false, int zMatAlphaFunc = 0);

	/** Binds the right shader for the given texture */
	void BindShaderForTextureDiffusePass(zCTexture* texture);


protected:
	/** List of visible vobs for the current frame */
	std::vector<VobInfo *> FrameVisibleVobs;
	std::vector<VobLightInfo *> FrameVisibleLights;

	D3D11PShader* PS_LPP;
};

