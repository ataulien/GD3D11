#pragma once
#include "basegraphicsengine.h"

struct RenderToDepthStencilBuffer;

class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class D3D11ShaderManager;

enum D3D11ENGINE_RENDER_STAGE
{
	DES_MAIN,
	DES_SHADOWMAP
};

const int DRAWVERTEXARRAY_BUFFER_SIZE = 2048 * sizeof(ExVertexStruct);
const int NUM_MAX_BONES = 96;
const int INSTANCING_BUFFER_SIZE = sizeof(D3DXMATRIX) * 1024;

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
struct MeshInfo;
struct RenderToTextureBuffer;
class D3D11GraphicsEngine : public BaseGraphicsEngine
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

	/** Returns the device */
	ID3D11Device* GetDevice(){ return Device; }

	/** Returns the context */
	ID3D11DeviceContext* GetContext(){ return Context; }

	/** Returns the deferred context */
	ID3D11DeviceContext* GetDeferredContext(){ return DeferredContext; }

	/** Gets the depthbuffer */
	RenderToDepthStencilBuffer* GetDepthBuffer(){return DepthStencilBuffer;}

	/** Returns the Backbuffers shader resource view */
	ID3D11ShaderResourceView* GetBackbufferSRV(){return BackbufferSRV;}

	/** Returns the first GBuffer */
	RenderToTextureBuffer* GetGBuffer0(){return GBuffer0_Diffuse;}

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

	XRESULT DrawWorldMeshW(bool noTextures=false);

	/** Draws the static VOBs */
	virtual XRESULT DrawVOBs(bool noTextures=false);

	/** Draws a single VOB */
	virtual void DrawVobSingle(VobInfo* vob);

	/** Draws everything around the given position */
	void DrawWorldAround(const D3DXVECTOR3& position, int sectionRange, float vobXZRange);

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
	void RenderShadowmaps(const D3DXVECTOR3& cameraPosition);

	/** Returns the shadermanager */
	D3D11ShaderManager* GetShaderManager();

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

	/** Draws underwater effects */
	void DrawUnderwaterEffects();

	/** Binds the right shader for the given texture */
	void BindShaderForTexture(zCTexture* texture, bool forceAlphaTest=false, int zMatAlphaFunc = 0);

	/** Copies the depth stencil buffer to DepthStencilBufferCopy */
	void CopyDepthStencil();

	/** Draws particle effects */
	void DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info);
protected:

	/** Creates the main UI-View */
	void CreateMainUIView();

	/** Test draw world */
	void TestDrawWorldMesh();

	/** D3D11 Objects */
	IDXGIFactory* DXGIFactory;
	IDXGIAdapter* DXGIAdapter;
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11DeviceContext* DeferredContext;
	ID3D11SamplerState* DefaultSamplerState;
	ID3D11SamplerState* ClampSamplerState;
	ID3D11SamplerState* CubeSamplerState;
	ID3D11SamplerState* ShadowmapSamplerState;
	ID3D11RasterizerState* WorldRasterizerState;
	ID3D11RasterizerState* HUDRasterizerState;
	ID3D11DepthStencilState* DefaultDepthStencilState;
	INT2 Resolution;

	/** FixedFunction-State render states */
	ID3D11RasterizerState* FFRasterizerState;
	ID3D11BlendState* FFBlendState;
	ID3D11DepthStencilState* FFDepthStencilState;

	/** Swapchain buffers */
	ID3D11RenderTargetView* BackbufferRTV;
	ID3D11ShaderResourceView* BackbufferSRV; // Diffuse
	RenderToTextureBuffer* GBuffer0_Diffuse;
	RenderToTextureBuffer* GBuffer1_Normals_SpecIntens_SpecPower; // Normals / SpecIntensity / SpecPower
	RenderToTextureBuffer* HDRBackBuffer;
	RenderToDepthStencilBuffer* DepthStencilBuffer;
	RenderToTextureBuffer* DepthStencilBufferCopy;

	/** Output window */
	HWND OutputWindow;

	/** ShaderManager */
	D3D11ShaderManager* ShaderManager;

	/** Temp-Arrays for storing data to be put in constant buffers */
	D3DXMATRIX Temp2D3DXMatrix[2];
	D3DXMATRIX TempBonesD3DXmatrix[NUM_MAX_BONES];
	float2 Temp2Float2[2];
	D3D11VertexBuffer* TempVertexBuffer;
	D3D11VertexBuffer* DynamicInstancingBuffer;

	/** Active shaders */
	D3D11VShader* ActiveVS;
	D3D11PShader* ActivePS;
	D3D11HDShader* ActiveHDS;
	std::set<zCTexture*> FrameTextures;

	/** Useful shaders */
	D3D11PShader* PS_DiffuseNormalmapped;
	D3D11PShader* PS_Diffuse;
	D3D11PShader* PS_DiffuseNormalmappedAlphatest;
	D3D11PShader* PS_DiffuseAlphatest;
	D3D11PShader* PS_Simple;

	/** Post processing */
	D3D11PfxRenderer* PfxRenderer;

	/** Sky */
	RenderToTextureBuffer* CloudBuffer;
	BaseTexture* DistortionTexture;
	BaseTexture* NoiseTexture;

	/** Lighting */
	GMesh* InverseUnitSphereMesh;

	/** Shadowing */
	RenderToDepthStencilBuffer* WorldShadowmap1;
	std::list<VobInfo*> RenderedVobs;

	/** Debugging */
	D3D11LineRenderer* LineRenderer;

	/** The current rendering stage */
	D3D11ENGINE_RENDER_STAGE RenderingStage;

	/** The editorcontrols */
	D2DView* UIView;

	/** If true, a present is still pending, so dont render anything */
	bool PresentPending;

	/** Map of texture/index */
	stdext::hash_map<zCTexture*, int> TexArrayIndexByTexture;

	/** List of water surfaces for this frame */
	std::hash_map<zCTexture*, std::vector<MeshInfo*>> FrameWaterSurfaces;

	/** Reflection */
	ID3D11ShaderResourceView* ReflectionCube;

	/** Constantbuffers for view-distances */
	D3D11ConstantBuffer* InfiniteRangeConstantBuffer;
	D3D11ConstantBuffer* OutdoorSmallVobsConstantBuffer;
	D3D11ConstantBuffer* OutdoorVobsConstantBuffer;

	/** Quads for decals/particles */
	BaseVertexBuffer* QuadVertexBuffer;
	BaseVertexBuffer* QuadIndexBuffer;
};