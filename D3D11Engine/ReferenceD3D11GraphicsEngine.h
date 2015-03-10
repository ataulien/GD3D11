#pragma once
#include "basegraphicsengine.h"

struct RenderToDepthStencilBuffer;

class ReferenceD3D11ConstantBuffer;
class ReferenceD3D11Shader;
class ReferenceD3D11GraphicsEngine :
	public BaseGraphicsEngine
{
public:
	ReferenceD3D11GraphicsEngine(void);
	~ReferenceD3D11GraphicsEngine(void);

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

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList(std::vector<DisplayModeInfo>* modeList);

	/** Presents the current frame to the screen */
	virtual XRESULT Present();

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices);

	/** Draws a vertexbuffer, non-indexed */
	virtual XRESULT DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices);

	/** Draws a skeletal mesh */
	virtual XRESULT DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms);

	/** Draws a vertexarray, non-indexed */
	virtual XRESULT DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices);

	/** Draws a batch of instanced geometry */
	virtual XRESULT DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances){return XR_SUCCESS;}
		
	/** Draws a triangle */
	void DrawTriangle();

	/** Returns the device */
	ID3D11Device* GetDevice(){return Device;}

	/** Returns the context */
	ID3D11DeviceContext* GetContext(){return Context;}

	/** Returns the deferred context */
	ID3D11DeviceContext* GetDeferredContext(){return DeferredContext;}

	/** Unbinds the texture at the given slot */
	virtual XRESULT UnbindTexture(int slot);

private:
	/** Recreates the renderstates */
	XRESULT UpdateRenderStates();

	/** D3D11 Objects */
	IDXGIFactory* DXGIFactory;
	IDXGIAdapter* DXGIAdapter;
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11DeviceContext* DeferredContext;
	ID3D11SamplerState* DefaultSamplerState;
	ID3D11RasterizerState* WorldRasterizerState;
	ID3D11RasterizerState* HUDRasterizerState;
	ID3D11DepthStencilState* DefaultDepthStencilState;

	/** FixedFunction-State render states */
	ID3D11RasterizerState* FFRasterizerState;
	ID3D11BlendState* FFBlendState;
	ID3D11DepthStencilState* FFDepthStencilState;

	/** Swapchain buffers */
	ID3D11RenderTargetView* BackbufferRTV;
	RenderToDepthStencilBuffer* DepthStencilBuffer;

	/** Output window */
	HWND OutputWindow;

	/** Test stuff */
	ReferenceD3D11Shader* SimpleShader;
	ReferenceD3D11Shader* SimpleSkeletalShader;
	ReferenceD3D11Shader* SimpleHUDShader;
	ReferenceD3D11ConstantBuffer* VS_ExConstantBuffer;
	ReferenceD3D11ConstantBuffer* VS_PerObjectExConstantBuffer;
	ReferenceD3D11ConstantBuffer* VS_ExSkelTransformConstantBuffer;

	/** Buffer for the DrawVertexArrayFunction */
	BaseVertexBuffer* DrawVertexArrayBuffer;
	ReferenceD3D11ConstantBuffer* VS_TransformedExConstantBuffer;

};

