#pragma once
#include "pch.h"
#include "WorldConverter.h"

class BaseVertexBuffer;
class BaseTexture;
class BaseConstantBuffer;
class BaseLineRenderer;
struct VobInfo;

struct DisplayModeInfo
{
	DWORD Height;
	DWORD Width;
	DWORD Bpp;
};

struct ViewportInfo
{
	unsigned int TopLeftX;
	unsigned int TopLeftY;
	unsigned int Width;
	unsigned int Height;
	float MinZ;
	float MaxZ;
};

class zCTexture;

/** Base graphics engine */
class BaseGraphicsEngine
{
public:
	enum EUIEvent
	{
		UI_OpenSettings,
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
	virtual XRESULT CreateVertexBuffer(BaseVertexBuffer** outBuffer) = 0;

	/** Creates a texture object (Not registered inside) */
	virtual XRESULT CreateTexture(BaseTexture** outTexture) = 0;

	/** Creates a constantbuffer object (Not registered inside) */
	virtual XRESULT CreateConstantBuffer(BaseConstantBuffer** outCB, void* data, int size) = 0;

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling = false) = 0;

	/** Presents the current frame to the screen */
	virtual XRESULT Present() = 0;

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int stride = sizeof(ExVertexStruct)) = 0;

	/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
	virtual XRESULT DrawVertexBufferFF(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride = sizeof(ExVertexStruct)) = 0;

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset = 0) = 0;
	virtual XRESULT DrawVertexBufferIndexedUINT(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset) = 0;

	/** Draws a skeletal mesh */
	virtual XRESULT DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms, float fatness = 1.0f, SkeletalMeshVisualInfo* msh = NULL) = 0;

	/** Draws a vertexarray, non-indexed */
	virtual XRESULT DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex = 0, unsigned int stride = sizeof(ExVertexStruct)) = 0;

	/** Draws a batch of instanced geometry */
	virtual XRESULT DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct)) = 0;
	virtual XRESULT DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, BaseVertexBuffer* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride = sizeof(ExVertexStruct), unsigned int startInstanceNum = 0, unsigned int indexOffset = 0) = 0;

	/** Sets the active pixel shader object */
	virtual XRESULT SetActivePixelShader(const std::string& shader) = 0;
	virtual XRESULT SetActiveVertexShader(const std::string& shader) = 0;

	/** Binds the active PixelShader */
	virtual XRESULT BindActivePixelShader() = 0;
	virtual XRESULT BindActiveVertexShader() = 0;

	/** Binds viewport information to the given constantbuffer slot */
	virtual XRESULT BindViewportInformation(const std::string& shader, int slot) = 0;

	/** Unbinds the texture at the given slot */
	virtual XRESULT UnbindTexture(int slot) = 0;

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering() = 0;

	/** Draws the world mesh */
	virtual XRESULT DrawWorldMesh(bool noTextures = false) = 0;

	/** Draws the static VOBs */
	virtual XRESULT DrawVOBs(bool noTextures=false) = 0;

	/** Draws the sky using the GSky-Object */
	virtual XRESULT DrawSky() = 0;

	/** Called when a key got pressed */
	virtual XRESULT OnKeyDown(unsigned int key) = 0;

	/** Returns the current resolution */
	virtual INT2 GetResolution() = 0;
	virtual INT2 GetBackbufferResolution() = 0;
	
	/** Returns the data of the backbuffer */
	virtual void GetBackbufferData(byte** data, int& pixelsize) = 0;

	/** Returns the line renderer object */
	virtual BaseLineRenderer* GetLineRenderer() = 0;

	/** Returns the textures drawn this frame */
	virtual const std::set<zCTexture*> GetFrameTextures() = 0;

	/** Draws a fullscreenquad, copying the given texture to the viewport */
	virtual void DrawQuad(INT2 position, INT2 size) = 0;

	/** Draws a single VOB */
	virtual void DrawVobSingle(VobInfo* vob) = 0;

	/** Message-Callback for the main window */
	virtual LRESULT OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	/** Called when a vob was removed from the world */
	virtual XRESULT OnVobRemovedFromWorld(zCVob* vob) = 0;

	/** Reloads shaders */
	virtual XRESULT ReloadShaders() = 0;

	/** Draws the water surfaces */
	virtual void DrawWaterSurfaces() = 0;

	/** Handles an UI-Event */
	virtual void OnUIEvent(EUIEvent uiEvent) = 0;

	/** Draws particle effects */
	virtual void DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info) = 0;
};

