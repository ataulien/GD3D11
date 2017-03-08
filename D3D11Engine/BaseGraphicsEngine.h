#pragma once
#include "pch.h"
#include "WorldConverter.h"

class D3D11VertexBuffer;
class D3D11Texture;
class D3D11ConstantBuffer;
class BaseLineRenderer;
struct VobInfo;

struct DisplayModeInfo
{
	DWORD Height;
	DWORD Width;
	DWORD Bpp;
};

enum RenderStage
{
	STAGE_DRAW_WORLD = 0,
	STAGE_DRAW_SKELETAL = 1,
};

struct ViewportInfo
{
	ViewportInfo(){}
	ViewportInfo(	unsigned int topleftX, 
					unsigned int topleftY, 
					unsigned int width, 
					unsigned int height, 
					float minZ = 0.0f, 
					float maxZ = 1.0f)
	{
		TopLeftX = topleftX;
		TopLeftY = topleftY;
		Width = width;
		Height = height;
		MinZ = minZ;
		MaxZ = maxZ;
	}

	unsigned int TopLeftX;
	unsigned int TopLeftY;
	unsigned int Width;
	unsigned int Height;
	float MinZ;
	float MaxZ;
};

class zCTexture;
class D3D11PointLight;

/** Base graphics engine */
class BaseGraphicsEngine
{
public:
	enum EUIEvent
	{
		UI_OpenSettings,
		UI_OpenEditor
	};

	BaseGraphicsEngine(void);
	virtual ~BaseGraphicsEngine(void);

	/** Called after the fake-DDraw-Device got created */
	virtual XRESULT Init() = 0;

	/** Called when the game created its window */
	virtual XRESULT SetWindow(HWND hWnd) = 0;

	/** Called on window resize/resolution change */
	virtual XRESULT OnResize(INT2 newSize) = 0;

	/** Called when the game wants to render a new frame */
	virtual XRESULT OnBeginFrame() = 0;

	/** Called when the game ended it's frame */
	virtual XRESULT OnEndFrame() = 0;

	/** Called to set the current viewport */
	virtual XRESULT SetViewport(const ViewportInfo& viewportInfo) = 0;

	/** Called when the game wants to clear the bound rendertarget */
	virtual XRESULT Clear(const float4& color) = 0;

	/** Creates a vertexbuffer object (Not registered inside) */
	virtual XRESULT CreateVertexBuffer(D3D11VertexBuffer** outBuffer) = 0;

	/** Creates a texture object (Not registered inside) */
	virtual XRESULT CreateTexture(D3D11Texture** outTexture) = 0;

	/** Creates a constantbuffer object (Not registered inside) */
	virtual XRESULT CreateConstantBuffer(D3D11ConstantBuffer** outCB, void* data, int size) = 0;

	/** Creates a bufferobject for a shadowed point light for info */
	virtual XRESULT CreateShadowedPointLight(VobLightInfo* lightInfo, bool dynamic = false){return XR_SUCCESS;}

	/** Delete a bufferobject for a shadowed point light from info */
	virtual XRESULT DeleteShadowedPointLight(VobLightInfo* lightInfo) { return XR_SUCCESS; }

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling = false) = 0;

	/** Presents the current frame to the screen */
	virtual XRESULT Present() = 0;

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering() = 0;

	/** Returns the line renderer object */
	virtual BaseLineRenderer* GetLineRenderer() = 0;

	/** Returns the graphics-device this is running on */
	virtual std::string GetGraphicsDeviceName() = 0;

	/** Draws a vertexarray, used for rendering gothics UI */
	virtual XRESULT DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex = 0, unsigned int stride = sizeof(ExVertexStruct)) = 0;

	/** Puts the current world matrix into a CB and binds it to the given slot */
	virtual void SetupPerInstanceConstantBuffer(int slot=1){};

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBuffer(D3D11VertexBuffer* vb, unsigned int numVertices, unsigned int stride = sizeof(ExVertexStruct)){return XR_SUCCESS;};

	/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
	virtual XRESULT DrawVertexBufferFF(D3D11VertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride = sizeof(ExVertexStruct)){return XR_SUCCESS;};

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBufferIndexed(D3D11VertexBuffer* vb, D3D11VertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset = 0){return XR_SUCCESS;};
	virtual XRESULT DrawVertexBufferIndexedUINT(D3D11VertexBuffer* vb, D3D11VertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset){return XR_SUCCESS;};

	/** Draws a skeletal mesh */
	virtual XRESULT DrawSkeletalMesh(D3D11VertexBuffer* vb, D3D11VertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms, float fatness = 1.0f, SkeletalMeshVisualInfo* msh = NULL){return XR_SUCCESS;};

	

	/** Draws a vertexarray, non-indexed */
	virtual XRESULT DrawIndexedVertexArray(ExVertexStruct* vertices, unsigned int numVertices, D3D11VertexBuffer* ib, unsigned int numIndices, unsigned int stride = sizeof(ExVertexStruct)){return XR_SUCCESS;};

	/** Draws a batch of instanced geometry */
	virtual XRESULT DrawInstanced(D3D11VertexBuffer* vb, D3D11VertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct)){return XR_SUCCESS;};
	virtual XRESULT DrawInstanced(D3D11VertexBuffer* vb, D3D11VertexBuffer* ib, unsigned int numIndices, D3D11VertexBuffer* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct), unsigned int startInstanceNum = 0, unsigned int indexOffset = 0){return XR_SUCCESS;};

	/** Sets the active pixel shader object */
	virtual XRESULT SetActivePixelShader(const std::string& shader){return XR_SUCCESS;};
	virtual XRESULT SetActiveVertexShader(const std::string& shader){return XR_SUCCESS;};

	/** Binds the active PixelShader */
	virtual XRESULT BindActivePixelShader(){return XR_SUCCESS;};
	virtual XRESULT BindActiveVertexShader(){return XR_SUCCESS;};

	/** Binds viewport information to the given constantbuffer slot */
	virtual XRESULT BindViewportInformation(const std::string& shader, int slot){return XR_SUCCESS;};

	/** Unbinds the texture at the given slot */
	virtual XRESULT UnbindTexture(int slot){return XR_SUCCESS;};

	/** Draws the world mesh */
	virtual XRESULT DrawWorldMesh(bool noTextures = false){return XR_SUCCESS;};

	/** Draws the static VOBs */
	virtual XRESULT DrawVOBs(bool noTextures=false){return XR_SUCCESS;};

	/** Draws the sky using the GSky-Object */
	virtual XRESULT DrawSky(){return XR_SUCCESS;};

	/** Called when a key got pressed */
	virtual XRESULT OnKeyDown(unsigned int key){return XR_SUCCESS;};

	/** Returns the current resolution */
	virtual INT2 GetResolution(){return INT2(0,0);};
	virtual INT2 GetBackbufferResolution(){return GetResolution();};
	
	/** Returns the data of the backbuffer */
	virtual void GetBackbufferData(byte** data, int& pixelsize){};

	/** Returns the textures drawn this frame */
	virtual const std::set<zCTexture*> GetFrameTextures(){return std::set<zCTexture*>();};

	/** Draws a fullscreenquad, copying the given texture to the viewport */
	virtual void DrawQuad(INT2 position, INT2 size){};

	/** Draws a single VOB */
	virtual void DrawVobSingle(VobInfo* vob){};

	/** Message-Callback for the main window */
	virtual LRESULT OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){return 0;};

	/** Called when a vob was removed from the world */
	virtual XRESULT OnVobRemovedFromWorld(zCVob* vob){return XR_SUCCESS;};

	/** Reloads shaders */
	virtual XRESULT ReloadShaders(){return XR_SUCCESS;};

	/** Draws the water surfaces */
	virtual void DrawWaterSurfaces(){};

	/** Handles an UI-Event */
	virtual void OnUIEvent(EUIEvent uiEvent){};

	/** Draws particle effects */
	virtual void DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info){};
};

