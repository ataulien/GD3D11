#pragma once
#include "D3D11GraphicsEngineBase.h"

struct RenderToDepthStencilBuffer;

class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class D3D11ShaderManager;

enum D3D11ENGINE_RENDER_STAGE
{
	DES_Z_PRE_PASS,
	DES_MAIN,
	DES_SHADOWMAP,
	DES_SHADOWMAP_CUBE
};

const int DRAWVERTEXARRAY_BUFFER_SIZE = 2048 * sizeof(ExVertexStruct);
const int NUM_MAX_BONES = 96;
const int INSTANCING_BUFFER_SIZE = sizeof(VobInstanceInfo) * 2048;

const int POINTLIGHT_SHADOWMAP_SIZE = 128;

class D3D11PointLight;
class D3D11VShader;
class D3D11PShader;
class D3D11PfxRenderer;
class D3D11LineRenderer;
class zCVobLight;
class zCVob;
class D2DView;
struct VobLightInfo;
class GMesh;
class GOcean;
class D3D11HDShader;
class D3D11OcclusionQuerry;
struct MeshInfo;
struct RenderToTextureBuffer;
class D3D11Effect;
class D3D11GraphicsEngine : public D3D11GraphicsEngineBase
{
public:
	D3D11GraphicsEngine(void);
	~D3D11GraphicsEngine(void);

	/** Called after the fake-DDraw-Device got created */
	virtual XRESULT Init();

	/** Called when the game created its window */
	virtual XRESULT SetWindow(HWND hWnd);

	/** Called on window resize/resolution change */
	virtual XRESULT OnResize(INT2 newSize);

	/** Called when the game wants to render a new frame */
	virtual XRESULT OnBeginFrame();

	/** Called when the game ended it's frame */
	virtual XRESULT OnEndFrame();

	/** Called to set the current viewport */
	virtual XRESULT SetViewport(const ViewportInfo& viewportInfo);

	/** Called when the game wants to clear the bound rendertarget */
	virtual XRESULT Clear(const float4& color);

	/** Creates a vertexbuffer object (Not registered inside) */
	virtual XRESULT CreateVertexBuffer(BaseVertexBuffer** outBuffer);

	/** Creates a texture object (Not registered inside) */
	virtual XRESULT CreateTexture(BaseTexture** outTexture);

	/** Creates a constantbuffer object (Not registered inside) */
	virtual XRESULT CreateConstantBuffer(BaseConstantBuffer** outCB, void* data, int size);

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling = false);

	/** Presents the current frame to the screen */
	virtual XRESULT Present();

	/** Saves a screenshot */
	virtual void SaveScreenshot();

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int stride = sizeof(ExVertexStruct));

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset = 0);
	virtual XRESULT DrawVertexBufferIndexedUINT(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset);

	/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
	virtual XRESULT DrawVertexBufferFF(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride = sizeof(ExVertexStruct));

	/** Binds viewport information to the given constantbuffer slot */
	virtual XRESULT BindViewportInformation(const std::string& shader, int slot);

	/** Sets up a draw call for a VS_Ex-Mesh */
	void SetupVS_ExMeshDrawCall();
	void SetupVS_ExConstantBuffer();
	void SetupVS_ExPerInstanceConstantBuffer();

	/** Puts the current world matrix into a CB and binds it to the given slot */
	void SetupPerInstanceConstantBuffer(int slot=1);

	enum EPNAENRenderMode
	{
		PNAEN_Default,
		PNAEN_Instanced,
		PNAEN_Skeletal,

	};
	/** Sets up everything for a PNAEN-Mesh */
	void Setup_PNAEN(EPNAENRenderMode mode = PNAEN_Default);

	/** Draws a skeletal mesh */
	virtual XRESULT DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms, float fatness = 1.0f, SkeletalMeshVisualInfo* msh= NULL);

	/** Draws a vertexarray, non-indexed */
	virtual XRESULT DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex = 0, unsigned int stride = sizeof(ExVertexStruct));

	/** Draws a vertexarray, indexed */
	virtual XRESULT DrawIndexedVertexArray(ExVertexStruct* vertices, unsigned int numVertices, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int stride = sizeof(ExVertexStruct));

	/** Draws a batch of instanced geometry */
	virtual XRESULT DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct));
	virtual XRESULT DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, BaseVertexBuffer* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct), unsigned int startInstanceNum = 0, unsigned int indexOffset = 0);

	/** Called when a vob was removed from the world */
	virtual XRESULT OnVobRemovedFromWorld(zCVob* vob);

	/** Called when a key got pressed */
	virtual XRESULT OnKeyDown(unsigned int key);

	/** Sets the active pixel shader object */
	virtual XRESULT SetActivePixelShader(const std::string& shader);
	virtual XRESULT SetActiveVertexShader(const std::string& shader);
	XRESULT SetActiveHDShader(const std::string& shader);

	/** Binds the active PixelShader */
	virtual XRESULT BindActivePixelShader();
	virtual XRESULT BindActiveVertexShader();

	/** Draws quadmarks in a simple way */
	void DrawQuadMarks();

	/** Draws the ocean */
	XRESULT DrawOcean(GOcean* ocean);

	/** Gets the depthbuffer */
	RenderToDepthStencilBuffer* GetDepthBuffer(){return DepthStencilBuffer;}

	/** Returns the Backbuffers shader resource view */
	ID3D11ShaderResourceView* GetBackbufferSRV(){return BackbufferSRV;}

	/** Returns the first GBuffer */
	RenderToTextureBuffer* GetGBuffer0(){return GBuffer0_Diffuse;}

	/** Returns the second GBuffer */
	RenderToTextureBuffer* GetGBuffer1(){return GBuffer1_Normals_SpecIntens_SpecPower;}

	/** Returns the HDRBackbuffer */
	RenderToTextureBuffer* GetHDRBackBuffer(){return HDRBackBuffer;}

	/** Unbinds the texture at the given slot */
	virtual XRESULT UnbindTexture(int slot);

	/** Sets up the default rendering state */
	void SetDefaultStates();

	/** Returns the current resolution (Maybe supersampled)*/
	INT2 GetResolution();
	
	/** Returns the actual resolution of the backbuffer (not supersampled) */
	INT2 GetBackbufferResolution();

	/** Returns the data of the backbuffer */
	void GetBackbufferData(byte** data, int& pixelsize);

	/** Returns the line renderer object */
	BaseLineRenderer* GetLineRenderer();

	/** ---------------- Gothic rendering functions -------------------- */

	/** Draws the world mesh */
	virtual XRESULT DrawWorldMesh(bool noTextures=false);

	/** Draws a list of mesh infos */
	XRESULT DrawMeshInfoListAlphablended(const std::vector<std::pair<MeshKey, MeshInfo*>>& list);

	XRESULT DrawWorldMeshW(bool noTextures=false);

	/** Draws the static VOBs */
	virtual XRESULT DrawVOBs(bool noTextures=false);

	/** Draws a single VOB */
	virtual void DrawVobSingle(VobInfo* vob);

	/** Draws everything around the given position */
	void DrawWorldAround(const D3DXVECTOR3& position, int sectionRange, float vobXZRange, bool cullFront = true, bool dontCull = false);
	void DrawWorldAround(const D3DXVECTOR3& position, 
					     float range,
					     bool cullFront = true, 
						 bool indoor = false,
						 bool noNPCs = false,
					     std::list<VobInfo*>* renderedVobs = NULL, std::list<SkeletalVobInfo*>* renderedMobs = NULL, std::map<MeshKey, WorldMeshInfo*, cmpMeshKey>* worldMeshCache = NULL);
					     
	/** Draws the static vobs instanced */
	XRESULT DrawVOBsInstanced();

	/** Applys the lighting to the scene */
	XRESULT DrawLighting(std::vector<VobLightInfo*>& lights);

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering();

	/** Draws the sky using the GSky-Object */
	virtual XRESULT DrawSky();

	/** Draws the 3D-Cloud-Sky */
	void Draw3DClouds();

	/** Renders the cloudmeshes */
	void DrawCloudMeshes();

	/** Renders the shadowmaps for the sun */
	void RenderShadowmaps(const D3DXVECTOR3& cameraPosition, RenderToDepthStencilBuffer* target = NULL, bool cullFront = true, bool dontCull = false, ID3D11DepthStencilView* dsvOverwrite = NULL, ID3D11RenderTargetView* debugRTV = NULL);

	/** Renders the shadowmaps for a pointlight */
	void RenderShadowCube(const D3DXVECTOR3& position, 
		float range, 
		RenderToDepthStencilBuffer* targetCube, 
		ID3D11DepthStencilView* face,
		ID3D11RenderTargetView* debugRTV = NULL, 
		bool cullFront = true, 
		bool indoor = false,
		bool noNPCs = false,
		std::list<VobInfo*>* renderedVobs = NULL, std::list<SkeletalVobInfo*>* renderedMobs = NULL, std::map<MeshKey, WorldMeshInfo*, cmpMeshKey>* worldMeshCache = NULL); 

	/** Updates the occlusion for the bsp-tree */
	void UpdateOcclusion();

	/** Recreates the renderstates */
	XRESULT UpdateRenderStates();

	/** Returns the textures drawn this frame */
	const std::set<zCTexture*> GetFrameTextures(){return FrameTextures;}

	/** Draws a fullscreenquad, copying the given texture to the viewport */
	void DrawQuad(INT2 position, INT2 size);

	/** Sets the current rendering stage */
	void SetRenderingStage(D3D11ENGINE_RENDER_STAGE stage);

	/** Returns the current rendering stage */
	D3D11ENGINE_RENDER_STAGE GetRenderingStage();

	/** Message-Callback for the main window */
	virtual LRESULT OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** Constructs the makro list for shader compilation */
	static void ConstructShaderMakroList(std::vector<D3D10_SHADER_MACRO>& list);

	/** Reloads shaders */
	virtual XRESULT ReloadShaders();

	/** Draws the given mesh infos as water */
	void DrawWaterSurfaces();

	/** Handles an UI-Event */
	void OnUIEvent(EUIEvent uiEvent);

	/** Draws the given list of decals */
	void DrawDecalList(const std::vector<zCVob *>& decals, bool lighting);

	/** Draws the particle-effects */
	void DrawParticleEffects();

	/** Draws the particle-effects using the geometry shader */
	void DrawParticleEffectsGS();

	/** Draws underwater effects */
	void DrawUnderwaterEffects();

	/** Binds the right shader for the given texture */
	void BindShaderForTexture(zCTexture* texture, bool forceAlphaTest=false, int zMatAlphaFunc = 0);

	/** Copies the depth stencil buffer to DepthStencilBufferCopy */
	void CopyDepthStencil();

	/** Draws particle effects */
	void DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info);

	/** Returns the UI-View */
	D2DView* GetUIView(){return UIView;}

	/** Creates the main UI-View */
	void CreateMainUIView();

	/** Returns a dummy cube-rendertarget used for pointlight shadowmaps */
	RenderToTextureBuffer* GetDummyCubeRT(){return DummyShadowCubemapTexture;}
protected:
	/** Test draw world */
	void TestDrawWorldMesh();

	D3D11PointLight* DebugPointlight;

	// Using a list here to determine which lights to update, since we don't want to update every light every frame.
	std::list<VobLightInfo*> FrameShadowUpdateLights;

	/** D3D11 Objects */
	ID3D11SamplerState* ClampSamplerState;
	ID3D11SamplerState* CubeSamplerState;
	ID3D11SamplerState* ShadowmapSamplerState;
	ID3D11RasterizerState* WorldRasterizerState;
	ID3D11RasterizerState* HUDRasterizerState;
	ID3D11DepthStencilState* DefaultDepthStencilState;

	/** Effects wrapper */
	D3D11Effect* Effects;

	/** Swapchain buffers */
	ID3D11RenderTargetView* BackbufferRTV;
	ID3D11ShaderResourceView* BackbufferSRV; // Diffuse
	RenderToTextureBuffer* GBuffer0_Diffuse;
	RenderToTextureBuffer* GBuffer1_Normals_SpecIntens_SpecPower; // Normals / SpecIntensity / SpecPower
	RenderToTextureBuffer* DepthStencilBufferCopy;
	RenderToTextureBuffer* DummyShadowCubemapTexture; // PS-Stage needs to have a rendertarget bound to execute SV_Depth-Writes, as it seems.

	/** Temp-Arrays for storing data to be put in constant buffers */
	D3DXMATRIX Temp2D3DXMatrix[2];
	D3DXMATRIX TempBonesD3DXmatrix[NUM_MAX_BONES];
	float2 Temp2Float2[2];
	D3D11VertexBuffer* DynamicInstancingBuffer;

	std::set<zCTexture*> FrameTextures;

	/** Post processing */
	D3D11PfxRenderer* PfxRenderer;

	/** Sky */
	RenderToTextureBuffer* CloudBuffer;
	BaseTexture* DistortionTexture;
	BaseTexture* NoiseTexture;
	BaseTexture* WhiteTexture;

	/** Lighting */
	GMesh* InverseUnitSphereMesh;

	/** Shadowing */
	RenderToDepthStencilBuffer* WorldShadowmap1;
	std::list<VobInfo*> RenderedVobs;

	/** The current rendering stage */
	D3D11ENGINE_RENDER_STAGE RenderingStage;

	/** The editorcontrols */
	D2DView* UIView;

	/** Map of texture/index */
	stdext::unordered_map<zCTexture*, int> TexArrayIndexByTexture;

	/** List of water surfaces for this frame */
	std::unordered_map<zCTexture*, std::vector<WorldMeshInfo*>> FrameWaterSurfaces;

	/** List of worldmeshes we have to render using alphablending */
	std::vector<std::pair<MeshKey, MeshInfo*>> FrameTransparencyMeshes;

	/** Reflection */
	ID3D11ShaderResourceView* ReflectionCube;
	ID3D11ShaderResourceView* ReflectionCube2;

	/** Constantbuffers for view-distances */
	D3D11ConstantBuffer* InfiniteRangeConstantBuffer;
	D3D11ConstantBuffer* OutdoorSmallVobsConstantBuffer;
	D3D11ConstantBuffer* OutdoorVobsConstantBuffer;

	/** Quads for decals/particles */
	BaseVertexBuffer* QuadVertexBuffer;
	BaseVertexBuffer* QuadIndexBuffer;

	/** Occlusion query manager */
	D3D11OcclusionQuerry* Occlusion;

	/** If true, we will save a screenshot after the next frame */
	bool SaveScreenshotNextFrame;
};