#include "pch.h"
#include "ReferenceD3D11GraphicsEngine.h"
#include "Logger.h"
#include "RenderToTextureBuffer.h"
#include "ReferenceD3D11VertexBuffer.h"
#include "ReferenceD3D11Shader.h"
#include "ReferenceD3D11ConstantBuffer.h"
#include "ReferenceD3D11Texture.h"
#include "Engine.h"
#include "GothicAPI.h"

//#define DEBUG_D3D11

const int DRAWVERTEXARRAY_BUFFER_SIZE = 512 * sizeof(ExVertexStruct);
const int NUM_MAX_BONES = 64;

struct VS_ExConstantBufferStruct
{
	D3DXMATRIX WorldView;
	D3DXMATRIX Proj;
};

struct VS_TransformedExConstantBufferStruct
{
	float2 ViewportPos;
	float2 ViewportSize;
};



ReferenceD3D11GraphicsEngine::ReferenceD3D11GraphicsEngine(void)
{
	SwapChain = NULL;
	DXGIFactory = NULL;
	DXGIAdapter = NULL;
	OutputWindow = NULL;
	BackbufferRTV = NULL;
	DepthStencilBuffer = NULL;
	Device = NULL;
	Context = NULL;
	DeferredContext = NULL;

	HUDRasterizerState = NULL;
	SimpleHUDShader = NULL;
	SimpleShader = NULL;
	SimpleSkeletalShader = NULL;
	VS_ExConstantBuffer = NULL;
	DefaultSamplerState = NULL;
	WorldRasterizerState = NULL;
	VS_TransformedExConstantBuffer = NULL;
	VS_PerObjectExConstantBuffer = NULL;
	VS_ExSkelTransformConstantBuffer = NULL;

	FFRasterizerState = NULL;
	FFBlendState = NULL;
	FFDepthStencilState = NULL;

	DefaultDepthStencilState = NULL;
}


ReferenceD3D11GraphicsEngine::~ReferenceD3D11GraphicsEngine(void)
{
	delete VS_ExConstantBuffer;
	delete DepthStencilBuffer;
	delete SimpleShader;
	delete VS_TransformedExConstantBuffer;
	delete VS_PerObjectExConstantBuffer;
	delete SimpleHUDShader;
	delete SimpleSkeletalShader;
	delete VS_ExSkelTransformConstantBuffer;

	if(DefaultDepthStencilState)DefaultDepthStencilState->Release();
	if(FFRasterizerState)FFRasterizerState->Release();
	if(FFBlendState)FFBlendState->Release();
	if(FFDepthStencilState)FFDepthStencilState->Release();

	if(HUDRasterizerState)HUDRasterizerState->Release();
	if(WorldRasterizerState)WorldRasterizerState->Release();
	if(DefaultSamplerState)DefaultSamplerState->Release();
	if(DeferredContext)DeferredContext->Release();
	if(BackbufferRTV)BackbufferRTV->Release();
	if(SwapChain)SwapChain->Release();
	if(Device)Device->Release();
	if(Context)Context->Release();
	if(DXGIAdapter)DXGIAdapter->Release();
	if(DXGIFactory)DXGIFactory->Release();
}

/** Called when the game created it's window */
XRESULT ReferenceD3D11GraphicsEngine::Init()
{
	HRESULT hr;

	LogInfo() << "Initializing Device...";

	// Create DXGI factory
	LE( CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&DXGIFactory) );
	LE( DXGIFactory->EnumAdapters(0, &DXGIAdapter) ); // Get first adapter

	// Find out what we are rendering on to write it into the logfile
	DXGI_ADAPTER_DESC adpDesc;
	DXGIAdapter->GetDesc(&adpDesc);
	std::wstring wdeviceDescription(adpDesc.Description);
	LogInfo() << "Rendering on: " << wdeviceDescription.c_str();

	// Create D3D11-Device
#ifndef DEBUG_D3D11
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, NULL, NULL, 0, D3D11_SDK_VERSION, &Device, NULL, &Context));
#else
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &Device, NULL, &Context));
#endif
	LE(Device->CreateDeferredContext(0, &DeferredContext)); // Used for multithreaded texture loading

	SimpleShader = new ReferenceD3D11Shader;
	XLE(SimpleShader->LoadShaders("system\\shaders\\VS_Ex.hlsl", "system\\shaders\\PS_Simple.hlsl"));

	SimpleHUDShader = new ReferenceD3D11Shader;
	XLE(SimpleHUDShader->LoadShaders("system\\shaders\\VS_TransformedEx.hlsl", "system\\shaders\\PS_Simple.hlsl"));

	SimpleSkeletalShader = new ReferenceD3D11Shader;
	XLE(SimpleSkeletalShader->LoadShaders("system\\shaders\\VS_ExSkeletal.hlsl", "system\\shaders\\PS_Simple.hlsl", 3));

	VS_ExConstantBufferStruct vsExDummy;
	VS_ExConstantBuffer = new ReferenceD3D11ConstantBuffer(sizeof(vsExDummy), &vsExDummy);

	VS_TransformedExConstantBufferStruct vsTrExDummy;
	VS_TransformedExConstantBuffer = new ReferenceD3D11ConstantBuffer(sizeof(vsTrExDummy), &vsTrExDummy);


	VS_PerObjectExConstantBuffer = new ReferenceD3D11ConstantBuffer(sizeof(vsTrExDummy), &vsTrExDummy);
	

	D3DXMATRIX temp[NUM_MAX_BONES];
	VS_ExSkelTransformConstantBuffer = new ReferenceD3D11ConstantBuffer(sizeof(D3DXMATRIX) * NUM_MAX_BONES, temp);


	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[ 0 ] = 1.0f;
	samplerDesc.BorderColor[ 1 ] = 1.0f;
	samplerDesc.BorderColor[ 2 ] = 1.0f;
	samplerDesc.BorderColor[ 3 ] = 1.0f;
	samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
	samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX
	Device->CreateSamplerState(&samplerDesc, &DefaultSamplerState);

	Context->PSSetSamplers(0, 1, &DefaultSamplerState);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true; // Gothics world vertices are CCW. That get's set by the GAPI-Graphics-State as well but is hardcoded here
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	Device->CreateRasterizerState(&rasterizerDesc, &WorldRasterizerState);
	Context->RSSetState(WorldRasterizerState);

	rasterizerDesc.FrontCounterClockwise = false;
	Device->CreateRasterizerState(&rasterizerDesc, &HUDRasterizerState);



	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	// Depth test parameters
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	Device->CreateDepthStencilState(&depthStencilDesc, &DefaultDepthStencilState);

	
	return XR_SUCCESS;
}

/** Called when the game created its window */
XRESULT ReferenceD3D11GraphicsEngine::SetWindow(HWND hWnd)
{
	LogInfo() << "Creating swapchain";
	OutputWindow = hWnd;

	OnResize(INT2(1920, 1080));

	return XR_SUCCESS;
}

/** Called on window resize/resolution change */
XRESULT ReferenceD3D11GraphicsEngine::OnResize(INT2 newSize)
{
	HRESULT hr;

	SetWindowPos(OutputWindow, NULL, 0, 0, newSize.x, newSize.y, 0);

	// Release all referenced buffer resources before we can resize the swapchain
	if(BackbufferRTV)BackbufferRTV->Release();
	delete DepthStencilBuffer;

	if(!SwapChain)
	{
		LogInfo() << "Creating new swapchain!";

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.BufferCount = 1;                                   
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;    
		scd.OutputWindow = OutputWindow;                           
		scd.SampleDesc.Count = 1;                            
		scd.SampleDesc.Quality = 0;

#ifdef PUBLIC_RELEASE
		scd.Windowed = false;                                  
#else
		scd.Windowed = true;                                   
#endif
		scd.BufferDesc.Height = newSize.y;
		scd.BufferDesc.Width = newSize.x;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		LE( DXGIFactory->CreateSwapChain(Device, &scd, &SwapChain));

		if(!SwapChain)
		{
			LogError() << "Failed to create Swapchain! Program will now exit!";
			exit(0);
		}
	}else
	{
		LogInfo() << "Resizing swapchain";

		if(FAILED(SwapChain->ResizeBuffers(1, newSize.x, newSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			LogError() << "Failed to resize swapchain! Errocode: " << hr;
			return XR_FAILED;
		}
	}

	// Successfully resized swapchain, re-get buffers
	ID3D11Texture2D* backbuffer = NULL;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

	// Recreate RenderTargetView
	LE( Device->CreateRenderTargetView(backbuffer, nullptr, &BackbufferRTV) );
	backbuffer->Release();

	// Recreate DepthStencilBuffer
	DepthStencilBuffer = new RenderToDepthStencilBuffer(Device, newSize.x, newSize.y, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	// Bind our newly created resources
	Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)newSize.x;
	viewport.Height = (float)newSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &viewport);

	CreateVertexBuffer(&DrawVertexArrayBuffer);
	DrawVertexArrayBuffer->Init(NULL, DRAWVERTEXARRAY_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);

	return XR_SUCCESS;
}

/** Called when the game wants to render a new frame */
XRESULT ReferenceD3D11GraphicsEngine::OnBeginFrame()
{
	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	ID3D11CommandList* dc_cl = NULL;
	DeferredContext->FinishCommandList(true, &dc_cl);
	Engine::GAPI->LeaveResourceCriticalSection();
	if(dc_cl)
	{
		Context->ExecuteCommandList(dc_cl, true);
		dc_cl->Release();
	}

	

	return XR_SUCCESS;
}

/** Called when the game ended it's frame */
XRESULT ReferenceD3D11GraphicsEngine::OnEndFrame()
{
	Engine::GAPI->DrawWorldMeshNaive();

	Present();
	return XR_SUCCESS;
}

/** Draws a triangle */
void ReferenceD3D11GraphicsEngine::DrawTriangle()
{
	BaseVertexBuffer* vx;
	CreateVertexBuffer(&vx);
	((ReferenceD3D11VertexBuffer *)vx)->Init(NULL, 3 * sizeof(ExVertexStruct), BaseVertexBuffer::EBindFlags::B_VERTEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	((ReferenceD3D11VertexBuffer *)vx)->CreateTriangle();

	DrawVertexBuffer(vx, 3);

	delete vx;
}

/** Called when the game wants to clear the bound rendertarget */
XRESULT ReferenceD3D11GraphicsEngine::Clear(const float4& color)
{
	Context->ClearRenderTargetView(BackbufferRTV, (float *)&color);
	Context->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	return XR_SUCCESS;
}

/** Creates a vertexbuffer object (Not registered inside) */
XRESULT ReferenceD3D11GraphicsEngine::CreateVertexBuffer(BaseVertexBuffer** outBuffer)
{
	*outBuffer = new ReferenceD3D11VertexBuffer;
	return XR_SUCCESS;
}

/** Creates a texture object (Not registered inside) */
XRESULT ReferenceD3D11GraphicsEngine::CreateTexture(BaseTexture** outTexture)
{
	*outTexture = new ReferenceD3D11Texture;
	return XR_SUCCESS;
}

/** Returns a list of available display modes */
XRESULT ReferenceD3D11GraphicsEngine::GetDisplayModeList(std::vector<DisplayModeInfo>* modeList)
{
	HRESULT hr;
	UINT numModes = 0;
	DXGI_MODE_DESC* displayModes = NULL;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	IDXGIOutput* output;
	DXGIAdapter->EnumOutputs(0, &output);

	hr = output->GetDisplayModeList( format, 0, &numModes, NULL);

	displayModes = new DXGI_MODE_DESC[numModes]; 

	// Get the list
	hr = output->GetDisplayModeList( format, 0, &numModes, displayModes);

	for(unsigned int i=0;i<numModes;i++)
	{
		if(displayModes[i].Format != DXGI_FORMAT_R8G8B8A8_UNORM)
			continue;

		DisplayModeInfo info;
		info.Height = displayModes[i].Height;
		info.Width = displayModes[i].Width;
		info.Bpp = 32;

		modeList->push_back(info);
	}

	output->Release();

	return XR_SUCCESS;
}

/** Presents the current frame to the screen */
XRESULT ReferenceD3D11GraphicsEngine::Present()
{
	SwapChain->Present(0,0);
	return XR_SUCCESS;
}

/** Called to set the current viewport */
XRESULT ReferenceD3D11GraphicsEngine::SetViewport(const ViewportInfo& viewportInfo)
{
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = viewportInfo.TopLeftX;
	viewport.TopLeftY = viewportInfo.TopLeftY;
	viewport.Width = (float)viewportInfo.Width;
	viewport.Height = (float)viewportInfo.Height;
	viewport.MinDepth = viewportInfo.MinZ;
	viewport.MaxDepth = viewportInfo.MaxZ;

	Context->RSSetViewports(1, &viewport);

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed */
XRESULT ReferenceD3D11GraphicsEngine::DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices)
{
	UpdateRenderStates();

	//Context->RSSetState(WorldRasterizerState);
	//Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	
	VS_ExConstantBufferStruct cb;

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetRendererState()->TransformState.TransformProj;

	//D3DXMatrixRotationY(&world, index);
	//D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0,0,10), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));
	//D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(45), 800.0f / 600.0f, 1.0f, 100.0f);

	//D3DXMatrixTranspose(&cb.WorldViewProj, &(world * view * proj)); // World is the same as WorldView, Gothic never uses the viewmatrix!
	D3DXMatrixTranspose(&cb.WorldView, &world);
	D3DXMatrixTranspose(&cb.Proj, &proj);
	//cb.WorldViewProj = world * view * proj;
	//cb.WorldView = world * view;
	//cb.Proj = proj;


	VS_ExConstantBuffer->UpdateBuffer(&cb);
	VS_ExConstantBuffer->BindToVertexShader(0);

	SimpleShader->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset = 0;
	UINT uStride = sizeof(ExVertexStruct);
	ID3D11Buffer* buffer = ((ReferenceD3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers( 0, 1, &buffer, &uStride, &offset );

	//Draw the mesh
	Context->Draw(numVertices, 0 );

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed */
XRESULT ReferenceD3D11GraphicsEngine::DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices)
{
	Context->RSSetState(WorldRasterizerState);
	Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	
	VS_ExConstantBufferStruct cb;

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetRendererState()->TransformState.TransformProj;

	//D3DXMatrixTranspose(&cb.WorldViewProj, &(world * view * proj)); // World is the same as WorldView, Gothic never uses the viewmatrix!
	D3DXMatrixTranspose(&cb.WorldView, &world);
	D3DXMatrixTranspose(&cb.Proj, &proj);

	VS_ExConstantBuffer->UpdateBuffer(&cb);
	VS_ExConstantBuffer->BindToVertexShader(0);

	SimpleShader->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset = 0;
	UINT uStride = sizeof(ExVertexStruct);
	ID3D11Buffer* buffer = ((ReferenceD3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers( 0, 1, &buffer, &uStride, &offset );
	Context->IASetIndexBuffer(((ReferenceD3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	//Draw the mesh
	Context->DrawIndexed(numIndices, 0, 0);

	return XR_SUCCESS;
}

/** Unbinds the texture at the given slot */
XRESULT ReferenceD3D11GraphicsEngine::UnbindTexture(int slot)
{
	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Draws a vertexarray, non-indexed */
XRESULT ReferenceD3D11GraphicsEngine::DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices)
{
	UpdateRenderStates();

	ExVertexStruct data[DRAWVERTEXARRAY_BUFFER_SIZE];
	memcpy(data, vertices, sizeof(ExVertexStruct) * numVertices);
	DrawVertexArrayBuffer->UpdateBuffer(data);

	D3D11_VIEWPORT vp;
	UINT num = 1;

	Context->RSGetViewports(&num, &vp);

	// Update viewport information
	VS_TransformedExConstantBufferStruct cb;
	cb.ViewportPos.x = vp.TopLeftX;
	cb.ViewportPos.y = vp.TopLeftY;

	cb.ViewportSize.x = vp.Width;
	cb.ViewportSize.y = vp.Height;

	VS_TransformedExConstantBuffer->UpdateBuffer(&cb);
	VS_TransformedExConstantBuffer->BindToVertexShader(1);


	SimpleHUDShader->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset = 0;
	UINT uStride = sizeof(ExVertexStruct);
	ID3D11Buffer* buffer = ((ReferenceD3D11VertexBuffer *)DrawVertexArrayBuffer)->GetVertexBuffer();
	Context->IASetVertexBuffers( 0, 1, &buffer, &uStride, &offset );

	//Draw the mesh
	Context->Draw(numVertices, 0 );

	return XR_SUCCESS;
}

/** Draws a skeletal mesh */
XRESULT ReferenceD3D11GraphicsEngine::DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms)
{
	Context->RSSetState(WorldRasterizerState);
	Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	
	VS_ExConstantBufferStruct cb;

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetRendererState()->TransformState.TransformProj;

	//D3DXMatrixTranspose(&cb.WorldViewProj, &(world * view * proj)); // World is the same as WorldView, Gothic never uses the viewmatrix!
	D3DXMatrixTranspose(&cb.WorldView, &world);
	D3DXMatrixTranspose(&cb.Proj, &proj);

	VS_ExConstantBuffer->UpdateBuffer(&cb);
	VS_ExConstantBuffer->BindToVertexShader(0);

	D3DXMATRIX temp[NUM_MAX_BONES];
	memcpy(temp, &transforms[0], sizeof(D3DXMATRIX) * transforms.size());
	VS_ExSkelTransformConstantBuffer->UpdateBuffer(temp);
	VS_ExSkelTransformConstantBuffer->BindToVertexShader(1);

	SimpleSkeletalShader->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset = 0;
	UINT uStride = sizeof(ExSkelVertexStruct);
	ID3D11Buffer* buffer = ((ReferenceD3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers( 0, 1, &buffer, &uStride, &offset );
	Context->IASetIndexBuffer(((ReferenceD3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	//Draw the mesh
	Context->DrawIndexed(numIndices, 0, 0);

	return XR_SUCCESS;
}

/** Recreates the renderstates */
XRESULT ReferenceD3D11GraphicsEngine::UpdateRenderStates()
{
	if(Engine::GAPI->GetRendererState()->BlendStateDirty)
	{
		if(FFBlendState)FFBlendState->Release();

		GothicBlendStateInfo& bs = Engine::GAPI->GetRendererState()->BlendState;

		D3D11_BLEND_DESC blendDesc;
		// Set to default
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED|D3D11_COLOR_WRITE_ENABLE_BLUE|D3D11_COLOR_WRITE_ENABLE_GREEN;
		blendDesc.RenderTarget[0].SrcBlend = (D3D11_BLEND)bs.SrcBlend;
		blendDesc.RenderTarget[0].DestBlend = (D3D11_BLEND)bs.DestBlend;
		blendDesc.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)bs.BlendOp;
		blendDesc.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)bs.SrcBlendAlpha;
		blendDesc.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)bs.DestBlendAlpha;
		blendDesc.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)bs.BlendOpAlpha;

		// Default
		blendDesc.RenderTarget[0].BlendEnable=bs.BlendEnabled;
			
		Device->CreateBlendState(&blendDesc, &FFBlendState);

		Context->OMSetBlendState(FFBlendState, (float *)&D3DXVECTOR4(0,0,0,0), 0xFFFFFFFF);

		Engine::GAPI->GetRendererState()->BlendStateDirty = false;
	}

	
	if(Engine::GAPI->GetRendererState()->RasterizerStateDirty)
	{
		if(FFRasterizerState)FFRasterizerState->Release();

		GothicRasterizerStateInfo& rs = Engine::GAPI->GetRendererState()->RasterizerState;

		D3D11_RASTERIZER_DESC rasterizerDesc;
		rasterizerDesc.CullMode = (D3D11_CULL_MODE)rs.CullMode;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = rs.ZBias;
		rasterizerDesc.DepthBiasClamp = 0;
		rasterizerDesc.SlopeScaledDepthBias = 0;
		rasterizerDesc.DepthClipEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;

		Device->CreateRasterizerState(&rasterizerDesc, &FFRasterizerState);
		Context->RSSetState(FFRasterizerState);

		Engine::GAPI->GetRendererState()->RasterizerStateDirty = false;
	}
	
	if(Engine::GAPI->GetRendererState()->DepthStateDirty)
	{
		if(FFDepthStencilState)FFDepthStencilState->Release();

		GothicDepthBufferStateInfo& ds = Engine::GAPI->GetRendererState()->DepthState;

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

		// Depth test parameters
		depthStencilDesc.DepthEnable = ds.DepthBufferEnabled;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = (D3D11_COMPARISON_FUNC)ds.DepthBufferCompareFunc;

		// Stencil test parameters
		depthStencilDesc.StencilEnable = false;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		Device->CreateDepthStencilState(&depthStencilDesc, &FFDepthStencilState);
		Context->OMSetDepthStencilState(FFDepthStencilState, 0);

		Engine::GAPI->GetRendererState()->DepthStateDirty = false;
	}

	return XR_SUCCESS;
}
