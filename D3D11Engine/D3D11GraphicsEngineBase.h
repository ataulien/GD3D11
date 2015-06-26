#pragma once
#include "basegraphicsengine.h"

class D3D11DepthBufferState;
class D3D11BlendStateInfo;
class D3D11RasterizerStateInfo;
class D3D11PShader;
class D3D11VShader;
class D3D11HDShader;
class D3D11Texture;
class D3D11GShader;

struct D3D11PipelineState : public PipelineState
{
	D3D11PipelineState()
	{
		BlendState = NULL;
		RasterizerState = NULL;
		DepthStencilState = NULL;
		SamplerState = NULL;
		IndexBuffer = NULL;
		VertexShader = NULL;
		InputLayout = NULL;
		PixelShader = NULL;
		HullShader = NULL;
		DomainShader = NULL;
		GeometryShader = NULL;
		DepthStencilView = NULL;
		NumRenderTargetViews = 0;

		ZeroMemory(RenderTargetViews, sizeof(RenderTargetViews));
		ZeroMemory(Textures, sizeof(Textures));
	}

	D3D11PipelineState(const PipelineState& s) : PipelineState(s)
	{
		D3D11PipelineState& d = (D3D11PipelineState&)s;

		// TODO: This is sooo ugly!
		BlendState = d.BlendState;
		RasterizerState = d.RasterizerState;
		DepthStencilState = d.DepthStencilState;
		SamplerState = d.SamplerState;
		ConstantBuffersVS = d.ConstantBuffersVS;
		ConstantBuffersPS = d.ConstantBuffersPS;
		ConstantBuffersHDS = d.ConstantBuffersHDS;
		ConstantBuffersGS = d.ConstantBuffersGS;
		VertexBuffers = d.VertexBuffers;
		IndexBuffer = d.IndexBuffer;
		VertexShader = d.VertexShader;
		InputLayout = d.InputLayout;
		PixelShader = d.PixelShader;
		HullShader = d.HullShader;
		DomainShader = d.DomainShader;
		GeometryShader = d.GeometryShader;
		DepthStencilView = d.DepthStencilView;
		NumRenderTargetViews = d.NumRenderTargetViews;

		memcpy(RenderTargetViews, d.RenderTargetViews, sizeof(RenderTargetViews));
		memcpy(Textures, d.Textures, sizeof(Textures));
	}

	/** State objects */
	ID3D11BlendState* BlendState;
	ID3D11RasterizerState* RasterizerState;
	ID3D11DepthStencilState* DepthStencilState;
	ID3D11SamplerState* SamplerState;

	/** Buffers */
	std::vector<ID3D11Buffer*> ConstantBuffersVS;
	std::vector<ID3D11Buffer*> ConstantBuffersPS;
	std::vector<ID3D11Buffer*> ConstantBuffersHDS;
	std::vector<ID3D11Buffer*> ConstantBuffersGS;

	/** Vertex-buffers */
	std::vector<ID3D11Buffer*> VertexBuffers;

	/** Index-buffer */
	ID3D11Buffer* IndexBuffer;

	/** Shaders */
	ID3D11VertexShader* VertexShader;
	ID3D11InputLayout* InputLayout;
	ID3D11PixelShader* PixelShader;
	ID3D11HullShader* HullShader;
	ID3D11DomainShader* DomainShader;
	ID3D11GeometryShader* GeometryShader;

	/** Rendertargets */
	ID3D11RenderTargetView* RenderTargetViews[8];
	byte NumRenderTargetViews;
	ID3D11DepthStencilView* DepthStencilView;

	/** Texture samplers */
	ID3D11ShaderResourceView* Textures[8];
};

namespace D3D11ObjectIDs
{
	/** Map to get a texture by ID */
	__declspec(selectany) std::map<UINT16, D3D11Texture*> TextureByID;
	

	__declspec(selectany) std::map<UINT8, D3D11PShader *> PShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11VShader *> VShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11HDShader *> HDShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11GShader *> GShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11DepthBufferState*> DepthStateByID;

	__declspec(selectany) std::map<UINT8, D3D11BlendStateInfo*> BlendStateByID;

	__declspec(selectany) std::map<UINT8, D3D11RasterizerStateInfo*> RasterizerStateByID;

	__declspec(selectany) struct CounterStruct{
		CounterStruct()
		{
			memset(this, 0, sizeof(CounterStruct));
		}

		int PShadersCounter;
		int TextureCounter;
		int VShadersCounter;
		int HDShadersCounter;
		int GShadersCounter;
		int DepthStateCounter;
		int BlendStateCounter;
		int RasterizerCounter;
	} Counters;

};

struct RenderToTextureBuffer;
struct RenderToDepthStencilBuffer;
class D3D11ShaderManager;

class D3D11VertexBuffer;
class D3D11LineRenderer;
class D3D11ConstantBuffer;
class D3D11GraphicsEngineBase :
	public BaseGraphicsEngine
{
public:
	D3D11GraphicsEngineBase(void);
	~D3D11GraphicsEngineBase(void);

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

	/** Creates a bufferobject for a shadowed point light */
	virtual XRESULT CreateShadowedPointLight(ShadowedPointLight** outPL, VobLightInfo* lightInfo, bool dynamic = false);

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling = false);

	/** Presents the current frame to the screen */
	virtual XRESULT Present();

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering() ;

	/** Returns the line renderer object */
	virtual BaseLineRenderer* GetLineRenderer();

	/** Creates a pipeline state */
	virtual PipelineState* CreatePipelineState(const PipelineState* copy = NULL);

	/** Saves a screenshot */
	virtual void SaveScreenshot(){}

	/** Returns the shadermanager */
	D3D11ShaderManager* GetShaderManager();

	/** Draws a vertexarray, used for rendering gothics UI */
	virtual XRESULT DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex = 0, unsigned int stride = sizeof(ExVertexStruct));

	/** Returns the Device/Context */
	ID3D11Device* GetDevice(){return Device;}
	ID3D11DeviceContext* GetContext(){return Context;}
	ID3D11DeviceContext* GetDeferredContext(){return DeferredContext;}

	/** Returns the current resolution */
	virtual INT2 GetResolution(){return Resolution;};

	/** Recreates the renderstates */
	XRESULT UpdateRenderStates();

	/** Constructs the makro list for shader compilation */
	static void ConstructShaderMakroList(std::vector<D3D10_SHADER_MACRO>& list);

	/** Sets up the default rendering state */
	void SetDefaultStates();

	/** Sets up a draw call for a VS_Ex-Mesh */
	void SetupVS_ExMeshDrawCall();
	void SetupVS_ExConstantBuffer();
	void SetupVS_ExPerInstanceConstantBuffer();

	/** Puts the current world matrix into a CB and binds it to the given slot */
	void SetupPerInstanceConstantBuffer(int slot=1);

	/** Sets the active pixel shader object */
	virtual XRESULT SetActivePixelShader(const std::string& shader);
	virtual XRESULT SetActiveVertexShader(const std::string& shader);
	virtual XRESULT SetActiveHDShader(const std::string& shader);
	virtual XRESULT SetActiveGShader(const std::string& shader);

	/** Returns the transforms constantbuffer */
	BaseConstantBuffer* GetTransformsCB();

	/** Updates the transformsCB with new values from the GAPI */
	void UpdateTransformsCB();
protected:
	/** Device-objects */
	IDXGIFactory* DXGIFactory;
	IDXGIAdapter* DXGIAdapter;

	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11DeviceContext* DeferredContext;

	/** Swapchain and resources */
	IDXGISwapChain* SwapChain;
	RenderToTextureBuffer* Backbuffer;
	RenderToDepthStencilBuffer* DepthStencilBuffer;
	RenderToTextureBuffer* HDRBackBuffer;

	/** States */
	ID3D11SamplerState* DefaultSamplerState;

	/** Output-window (Gothics main window)*/
	HWND OutputWindow;

	/** Total resolution we are rendering at */
	INT2 Resolution;

	/** Shader manager */
	D3D11ShaderManager* ShaderManager;

	/** Dynamic buffer for vertex array rendering */
	D3D11VertexBuffer* TempVertexBuffer;

	/** Constantbuffers */
	D3D11ConstantBuffer* TransformsCB; // Holds View/Proj-Transforms

	/** Shaders */
	D3D11PShader* PS_DiffuseNormalmapped;
	D3D11PShader* PS_DiffuseNormalmappedFxMap;
	D3D11PShader* PS_Diffuse;
	D3D11PShader* PS_DiffuseNormalmappedAlphatest;
	D3D11PShader* PS_DiffuseNormalmappedAlphatestFxMap;
	D3D11PShader* PS_DiffuseAlphatest;
	D3D11PShader* PS_Simple;
	D3D11PShader* PS_LinDepth;
	D3D11VShader* VS_Ex;
	D3D11VShader* VS_ExInstancedObj;
	D3D11GShader* GS_Billboard;

	D3D11VShader* ActiveVS;
	D3D11PShader* ActivePS;
	D3D11HDShader* ActiveHDS;
	D3D11GShader* ActiveGS;

	/** FixedFunction-State render states */
	ID3D11RasterizerState* FFRasterizerState;
	size_t FFRasterizerStateHash;
	ID3D11BlendState* FFBlendState;
	size_t FFBlendStateHash;
	ID3D11DepthStencilState* FFDepthStencilState;
	size_t FFDepthStencilStateHash;

	/** Debug line-renderer */
	D3D11LineRenderer* LineRenderer;

	/** If true, we are still waiting for a present to happen. Don't draw everything twice! */
	bool PresentPending;
};

