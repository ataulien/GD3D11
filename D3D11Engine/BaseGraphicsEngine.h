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


struct PipelineState
{
	PipelineState()
	{
		ZeroMemory(BaseState.ConstantBuffersPS, sizeof(BaseState.ConstantBuffersPS));
		ZeroMemory(BaseState.ConstantBuffersVS, sizeof(BaseState.ConstantBuffersVS));
		ZeroMemory(BaseState.ConstantBuffersHDS, sizeof(BaseState.ConstantBuffersHDS));
		ZeroMemory(BaseState.ConstantBuffersGS, sizeof(BaseState.ConstantBuffersGS));
		ZeroMemory(BaseState.VertexBuffers, sizeof(BaseState.VertexBuffers));
		ZeroMemory(BaseState.StructuredBuffersVS, sizeof(BaseState.StructuredBuffersVS));
		
		BaseState.GShaderID = 0xFF;
		BaseState.HDShaderID = 0xFF;
		BaseState.VShaderID = 0xFF;
		BaseState.PShaderID = 0xFF;

		BaseState.TranspacenyMode = 0;
		BaseState.Depth = 0;
		BaseState.BlendStateID = 0xFF;
		BaseState.RasterizerStateID = 0xFF;
		BaseState.DepthStencilID = 0xFF;

		BaseState.NumIndices = 0;
		BaseState.NumVertices = 0;
		BaseState.NumTextures = 0;
		BaseState.NumInstances = 0;
		BaseState.BSPSkipState = false;
		BaseState.InstanceOffset = 0;

		memset(BaseState.TextureIDs, 0xFF, sizeof(BaseState.TextureIDs));

		ZeroMemory(BaseState.VertexStride, sizeof(BaseState.VertexStride));
		BaseState.IndexOffset = 0;
		BaseState.DrawCallType = DCT_DrawIndexed;

		SortItem.AssociatedState = this;

		TransientState = false;
	}

	PipelineState(const PipelineState& s)
	{
		BaseState = s.BaseState;
		SortItem = s.SortItem;
		SortItem.AssociatedState = this;

		TransientState = s.TransientState;
	}

	virtual ~PipelineState(){}

	enum EDrawCallType
	{
		DCT_DrawTriangleList,
		DCT_DrawIndexed,
		DCT_DrawInstanced,
		DCT_DrawIndexedInstanced
	};

	enum ETransparencyMode
	{
		TM_NONE = 0,
		TM_MASKED = 1,
		TM_BLEND = 2
	};

	/** Called after this state got drawn */
	void StateWasDrawn()
	{
		BaseState.BSPSkipState = false;

		// Free memory if wanted on draw
		if(TransientState)
			delete this;
	}

	struct BaseState_s
	{
		/** Sets a constantbuffer to all shaders */
		void SetCB(int slot, D3D11ConstantBuffer* cb)
		{
			ConstantBuffersPS[slot] = cb;
			ConstantBuffersVS[slot] = cb;
			ConstantBuffersHDS[slot] = cb;
			ConstantBuffersGS[slot] = cb;
		}

		/** If true, this won't be rendered
			This is a special flag for the BSP-Tree, so that vobs at the border
			of a node won't be drawn multiple times */
		bool BSPSkipState;

		// Texture IDs of this state
		UINT16 TextureIDs[8];
		byte NumTextures;

		/** Buffers */
		D3D11ConstantBuffer* ConstantBuffersPS[8];
		D3D11ConstantBuffer* ConstantBuffersVS[8];
		D3D11ConstantBuffer* ConstantBuffersHDS[8];
		D3D11ConstantBuffer* ConstantBuffersGS[8];

		/** Vertex-buffers */
		D3D11VertexBuffer* VertexBuffers[2];
		D3D11VertexBuffer* StructuredBuffersVS[1];
		UINT VertexStride[2];
		UINT IndexOffset;
		EDrawCallType DrawCallType;
		UINT IndexStride;

		UINT NumIndices;
		UINT NumVertices;
		UINT NumInstances;
		UINT InstanceOffset;

		/** Index-buffer */
		D3D11VertexBuffer* IndexBuffer;

		UINT8 PShaderID;
		UINT8 VShaderID;
		UINT8 HDShaderID;
		UINT8 GShaderID;

		UINT8 TranspacenyMode;

		UINT16 Depth;

		UINT16 BlendStateID;
		UINT16 RasterizerStateID;
		UINT16 DepthStencilID;
	} BaseState;

	struct State {
		UINT64 Build(const BaseState_s& state)
		{
			/*Depth = state.Depth & 0xFFFF;
			textureID = state.TextureIDs[0] & 0xFFFF;
			psID = state.PShaderID & 0xFF;
			vsID = state.VShaderID & 0xFF;
			blendID = state.BlendStateID & 0xFF;
			rastID = state.RasterizerStateID & 0xFF;
			depthStencilID = state.DepthStencilID & 0xFF;
			transparency = state.TranspacenyMode & 3;
			vertexBufferID = 0;//state.VertexBuffers[0]->G // TODO
			padding = 0;*/

			/*UINT64 v =  (transparency & 3 << 62) |
						(textureID & 0xFF << 46) |
						(vertexBufferID & 0xF << 38) |
						(Depth & 0xFFF << 14);
						//(psID & 0xF << 46) |
						//(vsID & 0xF << 46) |
						//(blendID & 0xF << 46) |
						//(rastID & 0xF << 46) |
						//(depthStencilID & 0xF << 46);*/

			UINT64 v = 0;//((state.TextureIDs[0] & 0xFFFF) << 32) | ((UINT32)state.VertexBuffers[0]);
			return v;
		}

		UINT padding : 6;
		UINT transparency : 2;
		UINT textureID : 16;

		UINT vertexBufferID : 8;

		UINT Depth : 24;

		UINT psID : 8;
		UINT vsID : 8;

		UINT blendID : 8;
		UINT rastID : 8;
		UINT depthStencilID : 8;	
	};

	struct PipelineSortItem
	{
		static bool cmp(const PipelineSortItem* x, const PipelineSortItem* y)
		{
			return y->stateValue < x->stateValue;
		}

		UINT64 Build(const BaseState_s& state);

		/** Bitfield to describe the state */
		UINT64 stateValue;

		/** The state associated with this info */
		PipelineState* AssociatedState;
	};

	/** Structure for sorting this using the state-key */
	PipelineSortItem SortItem;

	/** If true, memory will be freed after this state was drawn */
	bool TransientState;
};


class zCTexture;
class ShadowedPointLight;

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

	/** Creates a bufferobject for a shadowed point light */
	virtual XRESULT CreateShadowedPointLight(ShadowedPointLight** outPL, VobLightInfo* lightInfo, bool dynamic = false){return XR_SUCCESS;}

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

	/** Fills the associated state object using the given IDs */
	virtual void FillPipelineStateObject(PipelineState* state){};

	/** Draws a single pipeline-state */
	virtual void DrawPipelineState(const PipelineState* state){}

	/** Pushes a single pipeline-state into the renderqueue */
	virtual void PushPipelineState(PipelineState* state){}

	/** Flushes the renderqueue */
	virtual void FlushRenderQueue(bool sortQueue = true){};

	/** Clears the renderingqueue */
	virtual void ClearRenderingQueue(){}

	/** Returns the rendering-queue */
	virtual std::vector<PipelineState::PipelineSortItem*>& GetRenderQueue(){static std::vector<PipelineState::PipelineSortItem*> s; return s;}

	/** Creates a pipeline state */
	virtual PipelineState* CreatePipelineState(const PipelineState* copy = NULL){return NULL;}

	/** Binds a pipeline-state */
	virtual void BindPipelineState(const PipelineState* state){}

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

	/** Fills a pipeline-state with the default value for the current stage */
	virtual void SetupPipelineForStage(int stage, PipelineState* state){};

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

