#include "pch.h"
#include "D3D11GraphicsEngine.h"
#include <D3DX11.h>
#include "D3D11VertexBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11PShader.h"
#include "D3D11VShader.h"
#include "D3D11HDShader.h"
#include "D3D11GShader.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Texture.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "Logger.h"
#include "RenderToTextureBuffer.h"
#include "GSky.h"
#include "GMesh.h"
#include "D3D11PfxRenderer.h"
#include "ConstantBufferStructs.h"
#include "BaseAntTweakBar.h"
#include "zCVob.h"
#include "zCVobLight.h"
#include "zCMaterial.h"
#include "zCLightmap.h"
#include "zCBspTree.h"
#include "D3D11LineRenderer.h"
#include "D3D7\MyDirectDrawSurface7.h"
#include "zCView.h"
#include "D2DView.h"
#include "GOcean.h"
#include "D2DDialog.h"
#include "AlignedAllocator.h"
#include "zCDecal.h"
#include "zCQuadMark.h"
#include "D2DEditorView.h"
#include <algorithm>
#include "D3D11PipelineStates.h"
#include "zCParticleFX.h"
#include "win32ClipboardWrapper.h"
#include "D3D11OcclusionQuerry.h"
#include "ModSpecific.h"
#include "D3D11Effect.h"
#include "D3D11PointLight.h"

//#include "MemoryTracker.h"

#pragma comment( lib, "dxguid.lib")

const int RES_UPSCALE = 1;
const INT2 DEFAULT_RESOLUTION = INT2(1920 * RES_UPSCALE, 1080*  RES_UPSCALE);
//const int WORLD_SHADOWMAP_SIZE = 1024;

const int NUM_UNLOADEDTEXCOUNT_FORCE_LOAD_TEXTURES = 100;

const float DEFAULT_NORMALMAP_STRENGTH = 0.10f;
const float DEFAULT_FAR_PLANE = 50000.0f;
const D3DXVECTOR4 UNDERWATER_COLOR_MOD = D3DXVECTOR4(0.5f, 0.7f, 1.0f, 1.0f);

const float NUM_FRAME_SHADOW_UPDATES = 2; // Fraction of lights to update per frame
const int NUM_MIN_FRAME_SHADOW_UPDATES = 4; // Minimum lights to update per frame
const int MAX_IMPORTANT_LIGHT_UPDATES = 1;
//#define DEBUG_D3D11

#define RECORD_LAST_DRAWCALL
#ifdef RECORD_LAST_DRAWCALL
struct DrawcallInfo
{
	enum DrawCallType
	{
		VB=0,
		VB_IX=1,
		VB_IX_UINT=2,
	};

	DrawCallType Type;
	unsigned int NumElements;
	unsigned int BaseVertexLocation;
	unsigned int BaseIndexLocation;

	void Print()
	{
		LogInfo() << "Last Drawcall: " << Type << " NumElements: " << NumElements << " BaseVertexLocation: " << BaseVertexLocation << " BaseIndexLocation: " << BaseIndexLocation;
	}

	bool Check()
	{
		if(NumElements > 0xFFFF * 4)
		{
			LogWarn() << "Invalid amount of NumElements supplied to drawcall!";
			Print();
			return false;
		}

		return true;
	}
};
DrawcallInfo g_LastDrawCall;
#endif

D3D11GraphicsEngine::D3D11GraphicsEngine(void)
{
	Resolution = DEFAULT_RESOLUTION;
	
	DebugPointlight = NULL;

	SwapChain = NULL;
	DXGIFactory = NULL;
	DXGIAdapter = NULL;
	OutputWindow = NULL;
	BackbufferRTV = NULL;
	DepthStencilBuffer = NULL;
	Device = NULL;
	Context = NULL;
	ReflectionCube2 = NULL;
	DistortionTexture = NULL;
	DeferredContext = NULL;
	UIView = NULL;
	DepthStencilBufferCopy = NULL;
	DummyShadowCubemapTexture = NULL;

	InfiniteRangeConstantBuffer = NULL;
	OutdoorSmallVobsConstantBuffer = NULL;
	OutdoorVobsConstantBuffer = NULL;
	ShadowmapSamplerState = NULL;
	DefaultSamplerState = NULL;
	HUDRasterizerState = NULL;
	WorldRasterizerState = NULL;
	DefaultDepthStencilState = NULL;

	FFRasterizerState = NULL;
	FFBlendState = NULL;
	FFDepthStencilState = NULL;

	ActiveHDS = NULL;
	ShaderManager = NULL;
	DynamicInstancingBuffer = NULL;
	TempVertexBuffer = NULL;
	NoiseTexture = NULL;
	WhiteTexture = NULL;

	ActivePS = NULL;
	PfxRenderer = NULL;
	CloudBuffer = NULL;
	BackbufferSRV = NULL;

	InverseUnitSphereMesh = NULL;
	ReflectionCube = NULL;

	GBuffer1_Normals_SpecIntens_SpecPower = NULL;
	GBuffer0_Diffuse = NULL;
	WorldShadowmap1 = NULL;
	HDRBackBuffer = NULL;
	CubeSamplerState = NULL;
	ClampSamplerState = NULL;

	Effects = new D3D11Effect;

	RenderingStage = DES_MAIN;

	PresentPending = false;
	SaveScreenshotNextFrame = false;

	LineRenderer = new D3D11LineRenderer;
	Occlusion = new D3D11OcclusionQuerry;


	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);

	// Match the resolution with the current desktop resolution
	Resolution = Engine::GAPI->GetRendererState()->RendererSettings.LoadedResolution;
}

D3D11GraphicsEngine::~D3D11GraphicsEngine(void)
{
	GothicDepthBufferStateInfo::DeleteCachedObjects();
	GothicBlendStateInfo::DeleteCachedObjects();
	GothicRasterizerStateInfo::DeleteCachedObjects();

	delete Effects; Effects = NULL;
	delete Occlusion; Occlusion = NULL;
	delete InfiniteRangeConstantBuffer;InfiniteRangeConstantBuffer = NULL;
	delete OutdoorSmallVobsConstantBuffer;OutdoorSmallVobsConstantBuffer = NULL;
	delete OutdoorVobsConstantBuffer;OutdoorVobsConstantBuffer = NULL;

	delete DummyShadowCubemapTexture; DummyShadowCubemapTexture = NULL;
	delete QuadVertexBuffer;QuadVertexBuffer = NULL;
	delete QuadIndexBuffer;QuadIndexBuffer = NULL;

	delete LineRenderer;LineRenderer = NULL;
	delete DepthStencilBuffer;DepthStencilBuffer = NULL;
	delete DynamicInstancingBuffer;DynamicInstancingBuffer = NULL;
	delete PfxRenderer;PfxRenderer = NULL;
	delete CloudBuffer;CloudBuffer = NULL;
	delete DistortionTexture;DistortionTexture = NULL;
	delete WhiteTexture;WhiteTexture = NULL;
	delete NoiseTexture;NoiseTexture = NULL;
	delete ShaderManager;ShaderManager = NULL;
	delete TempVertexBuffer;TempVertexBuffer = NULL;
	delete GBuffer1_Normals_SpecIntens_SpecPower;GBuffer1_Normals_SpecIntens_SpecPower = NULL;
	delete GBuffer0_Diffuse;GBuffer0_Diffuse = NULL;
	delete HDRBackBuffer;HDRBackBuffer = NULL;
	delete WorldShadowmap1;WorldShadowmap1 = NULL;
	delete InverseUnitSphereMesh;InverseUnitSphereMesh = NULL;
	delete UIView;UIView = NULL;

	if(ReflectionCube2)ReflectionCube2->Release();
	if(ReflectionCube)ReflectionCube->Release();
	if (CubeSamplerState)CubeSamplerState->Release();
	if (ClampSamplerState)ClampSamplerState->Release();
	if (ShadowmapSamplerState)ShadowmapSamplerState->Release();
	if (FFRasterizerState)FFRasterizerState->Release();
	if (FFBlendState)FFBlendState->Release();
	if (FFDepthStencilState)FFDepthStencilState->Release();
	if (BackbufferSRV)BackbufferSRV->Release();
	if (DefaultSamplerState)DefaultSamplerState->Release();
	if (HUDRasterizerState)HUDRasterizerState->Release();
	if (WorldRasterizerState)WorldRasterizerState->Release();
	if (DefaultDepthStencilState)DefaultDepthStencilState->Release();
	
	if (BackbufferRTV)BackbufferRTV->Release();
	if (SwapChain)SwapChain->Release();
	
	if (Context)Context->Release();
	//if (DeferredContext)DeferredContext->Release();

	ID3D11Debug* d3dDebug;
	Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));

	if(d3dDebug)
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		d3dDebug->Release();
	}

	if(Device)Device->Release();
	
	if (DXGIAdapter)DXGIAdapter->Release();
	if (DXGIFactory)DXGIFactory->Release();

	//MemTrackerFinalReport();
}

/** Called when the game created it's window */
XRESULT D3D11GraphicsEngine::Init()
{
	HRESULT hr;

	LogInfo() << "Initializing Device...";

	// Create DXGI factory
	LE(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&DXGIFactory));
	LE(DXGIFactory->EnumAdapters(0, &DXGIAdapter)); // Get first adapter

	// Find out what we are rendering on to write it into the logfile
	DXGI_ADAPTER_DESC adpDesc;
	DXGIAdapter->GetDesc(&adpDesc);

	std::wstring wDeviceDescription(adpDesc.Description);
	std::string deviceDescription(wDeviceDescription.begin(), wDeviceDescription.end());
	DeviceDescription = deviceDescription;
	LogInfo() << "Rendering on: " << deviceDescription.c_str();

	int flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;

	// Create D3D11-Device
#ifndef DEBUG_D3D11
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, &featurelevel, 1, D3D11_SDK_VERSION, &Device, NULL, &Context));
#else
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags | D3D11_CREATE_DEVICE_DEBUG, &featurelevel, 1, D3D11_SDK_VERSION, &Device, NULL, &Context));
#endif
	
	if(hr == DXGI_ERROR_UNSUPPORTED)
	{
		LogErrorBox() <<	"Your GPU (" << deviceDescription.c_str() << ") does not support Direct3D 11, so it can't run GD3D11!\n"
			"It has to be at least Featurelevel 11_0 compatible, which requires at least:\n"
			" *	Nvidia GeForce GTX4xx or higher\n"
			" *	AMD Radeon 5xxx or higher\n\n"
			"The game will now close.";
		exit(0);
	}
	
	LE(Device->CreateDeferredContext(0, &DeferredContext)); // Used for multithreaded texture loading
	
	LogInfo() << "Creating ShaderManager";

	ShaderManager = new D3D11ShaderManager();
	ShaderManager->Init();
	ShaderManager->LoadShaders();

	PS_DiffuseNormalmapped = ShaderManager->GetPShader("PS_DiffuseNormalmapped");
	PS_Diffuse = ShaderManager->GetPShader("PS_Diffuse");
	PS_DiffuseNormalmappedAlphatest = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphaTest");
	PS_DiffuseAlphatest = ShaderManager->GetPShader("PS_DiffuseAlphaTest");


	TempVertexBuffer = new D3D11VertexBuffer();
	TempVertexBuffer->Init(NULL, DRAWVERTEXARRAY_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);

	DynamicInstancingBuffer = new D3D11VertexBuffer();
	DynamicInstancingBuffer->Init(NULL, INSTANCING_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);


	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
	samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

	LE(Device->CreateSamplerState(&samplerDesc, &DefaultSamplerState));
	Context->PSSetSamplers(0, 1, &DefaultSamplerState);
	Context->VSSetSamplers(0, 1, &DefaultSamplerState);
	Context->DSSetSamplers(0, 1, &DefaultSamplerState);
	Context->HSSetSamplers(0, 1, &DefaultSamplerState);

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	Device->CreateSamplerState(&samplerDesc, &ShadowmapSamplerState);
	Context->PSSetSamplers(2, 1, &ShadowmapSamplerState);
	Context->VSSetSamplers(2, 1, &ShadowmapSamplerState);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Device->CreateSamplerState(&samplerDesc, &ClampSamplerState);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Device->CreateSamplerState(&samplerDesc, &CubeSamplerState);

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

	LE(Device->CreateRasterizerState(&rasterizerDesc, &WorldRasterizerState));
	Context->RSSetState(WorldRasterizerState);

	rasterizerDesc.FrontCounterClockwise = false;
	LE(Device->CreateRasterizerState(&rasterizerDesc, &HUDRasterizerState));

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

	LE(Device->CreateDepthStencilState(&depthStencilDesc, &DefaultDepthStencilState));

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	CreateTexture(&DistortionTexture);
	DistortionTexture->Init("system\\GD3D11\\textures\\distortion2.dds");

	CreateTexture(&NoiseTexture);
	NoiseTexture->Init("system\\GD3D11\\textures\\noise.png");

	CreateTexture(&WhiteTexture);
	WhiteTexture->Init("system\\GD3D11\\textures\\white.png");
	
	InverseUnitSphereMesh = new GMesh;
	InverseUnitSphereMesh->LoadMesh("system\\GD3D11\\meshes\\icoSphere.obj");

	// Create distance-buffers
	CreateConstantBuffer((BaseConstantBuffer **)&InfiniteRangeConstantBuffer, NULL, sizeof(float4));
	CreateConstantBuffer((BaseConstantBuffer **)&OutdoorSmallVobsConstantBuffer, NULL, sizeof(float4));
	CreateConstantBuffer((BaseConstantBuffer **)&OutdoorVobsConstantBuffer, NULL, sizeof(float4));

	// Init inf-buffer now
	InfiniteRangeConstantBuffer->UpdateBuffer(&D3DXVECTOR4(FLT_MAX, 0, 0, 0));

	// Load reflectioncube
	if(S_OK != D3DX11CreateShaderResourceViewFromFile(Device, "system\\GD3D11\\Textures\\reflect_cube.dds", NULL, NULL, &ReflectionCube, NULL))
		LogWarn() << "Failed to load file: system\\GD3D11\\Textures\\reflect_cube.dds";

	if(S_OK != D3DX11CreateShaderResourceViewFromFile(Device, "system\\GD3D11\\Textures\\SkyCubemap2.dds", NULL, NULL, &ReflectionCube2, NULL))
		LogWarn() << "Failed to load file: system\\GD3D11\\Textures\\SkyCubemap2.dds";
	

	// Init quad buffers
	Engine::GraphicsEngine->CreateVertexBuffer(&QuadVertexBuffer);
	QuadVertexBuffer->Init(NULL, 6 * sizeof(ExVertexStruct), BaseVertexBuffer::EBindFlags::B_VERTEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);


	Engine::GraphicsEngine->CreateVertexBuffer(&QuadIndexBuffer);
	QuadIndexBuffer->Init(NULL, 6 * sizeof(VERTEX_INDEX), BaseVertexBuffer::EBindFlags::B_INDEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	
	ExVertexStruct vx[6];
	ZeroMemory(vx, sizeof(vx));

	float scale = 1.0f;
	vx[0].Position = float3(-scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[1].Position = float3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[2].Position = float3(-scale * 0.5f, scale * 0.5f, 0.0f);

	vx[0].TexCoord = float2(0,0);
	vx[1].TexCoord = float2(1,0);
	vx[2].TexCoord = float2(0,1);

	vx[0].Color = 0xFFFFFFFF;
	vx[1].Color = 0xFFFFFFFF;
	vx[2].Color = 0xFFFFFFFF;

	vx[3].Position = float3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[4].Position = float3(scale * 0.5f, scale * 0.5f, 0.0f);
	vx[5].Position = float3(-scale * 0.5f, scale * 0.5f, 0.0f);

	vx[3].TexCoord = float2(1,0);
	vx[4].TexCoord = float2(1,1);
	vx[5].TexCoord = float2(0,1);

	vx[3].Color = 0xFFFFFFFF;
	vx[4].Color = 0xFFFFFFFF;
	vx[5].Color = 0xFFFFFFFF;

	QuadVertexBuffer->UpdateBuffer(vx);

	VERTEX_INDEX indices[] = {0,1,2,3,4,5};
	QuadIndexBuffer->UpdateBuffer(indices, sizeof(indices));

	// Create dummy rendertarget for shadowcubes
	DummyShadowCubemapTexture = new RenderToTextureBuffer(Device, 
		POINTLIGHT_SHADOWMAP_SIZE,
		POINTLIGHT_SHADOWMAP_SIZE,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		NULL,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		1,
		6);

	return XR_SUCCESS;
}

/** Called when the game created its window */
XRESULT D3D11GraphicsEngine::SetWindow(HWND hWnd)
{
	LogInfo() << "Creating swapchain";
	OutputWindow = hWnd;

	INT2 res = Resolution;

#ifdef BUILD_SPACER
	RECT r;
	GetClientRect(hWnd, &r);

	res.x = r.right;
	res.y = r.bottom;
#endif

	if(res.x != 0 && res.y != 0)
		OnResize(res);

	return XR_SUCCESS;
}

/** Called on window resize/resolution change */
XRESULT D3D11GraphicsEngine::OnResize(INT2 newSize)
{
	HRESULT hr;

	if(memcmp(&Resolution, &newSize, sizeof(newSize)) == 0 && SwapChain)
		return XR_SUCCESS; // Don't resize if we don't have to

	Resolution = newSize;
	INT2 bbres = GetBackbufferResolution();

#ifndef BUILD_SPACER
	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	SetWindowPos(OutputWindow, NULL, 0, 0, desktopRect.right, desktopRect.bottom, 0);
#endif

	// Release all referenced buffer resources before we can resize the swapchain
	if (BackbufferRTV)BackbufferRTV->Release();
	if (BackbufferSRV)BackbufferSRV->Release();
	delete DepthStencilBuffer;
	DepthStencilBuffer = NULL;

	if(UIView)UIView->PrepareResize();

	if (!SwapChain)
	{
		LogInfo() << "Creating new swapchain! (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		scd.OutputWindow = OutputWindow;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferDesc.Height = bbres.y;
		scd.BufferDesc.Width = bbres.x;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		bool windowed = Engine::GAPI->HasCommandlineParameter("ZWINDOW") ||
						Engine::GAPI->GetIntParamFromConfig("zStartupWindowed");
		scd.Windowed = windowed;

#ifndef PUBLIC_RELEASE
		scd.Windowed = true;
#endif

#ifdef BUILD_GOTHIC_1_08k
#ifdef PUBLIC_RELEASE
		scd.Windowed = false;
#else
		scd.Windowed = true;
#endif
#endif


		LE(DXGIFactory->CreateSwapChain(Device, &scd, &SwapChain));
		 
		if (!SwapChain)
		{
			LogError() << "Failed to create Swapchain! Program will now exit!";
			exit(0);
		}

		// Need to init AntTweakBar now that we have a working swapchain
		XLE(Engine::AntTweakBar->Init());

	}
	else
	{
		LogInfo() << "Resizing swapchain  (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		if (FAILED(SwapChain->ResizeBuffers(1, bbres.x, bbres.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			LogError() << "Failed to resize swapchain!";
			return XR_FAILED;
		}
	}

	// Successfully resized swapchain, re-get buffers
	ID3D11Texture2D* backbuffer = NULL;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

	// Recreate RenderTargetView
	LE(Device->CreateRenderTargetView(backbuffer, nullptr, &BackbufferRTV));
	LE(Device->CreateShaderResourceView(backbuffer, nullptr, &BackbufferSRV));

	if(UIView)
	{
		UIView->Resize(Resolution, backbuffer);
	}

	backbuffer->Release();

	// Recreate DepthStencilBuffer
	delete DepthStencilBuffer;
	DepthStencilBuffer = new RenderToDepthStencilBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	delete DepthStencilBufferCopy;
	DepthStencilBufferCopy = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	// Bind our newly created resources
	Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)bbres.x;
	viewport.Height = (float)bbres.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &viewport);

	
	// Create PFX-Renderer
	if(!PfxRenderer)
		PfxRenderer = new D3D11PfxRenderer;

	PfxRenderer->OnResize(Resolution);

	delete CloudBuffer;
	CloudBuffer = new RenderToTextureBuffer(Device, bbres.x, bbres.y, DXGI_FORMAT_R8G8B8A8_UNORM);

	delete GBuffer1_Normals_SpecIntens_SpecPower;
	GBuffer1_Normals_SpecIntens_SpecPower = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT);

	delete GBuffer0_Diffuse;
	GBuffer0_Diffuse = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	delete HDRBackBuffer;
	HDRBackBuffer = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT);

	delete WorldShadowmap1;
	int s = Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize;
	WorldShadowmap1 = new RenderToDepthStencilBuffer(Device, s, s, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	Engine::AntTweakBar->OnResize(newSize);
	

	return XR_SUCCESS;
}

/** Called when the game wants to render a new frame */
XRESULT D3D11GraphicsEngine::OnBeginFrame()
{
	Engine::GAPI->GetRendererState()->RendererInfo.Timing.StartTotal();

	static bool s_firstFrame = true;
	if(s_firstFrame)
	{
		// Check for normalmaps here, next release won't be using this file at all anyways...
		if(!ModSpecific::NormalmapPackageInstalled(ModSpecific::NRMPACK_ORIGINAL) || 
			!ModSpecific::NormalmapPackageInstalled(ModSpecific::GetModNormalmapPackName()))
		{
			LogInfo() << "Normalmaps missing, downloading now...";

			CreateMainUIView();

			if(UIView)
			{
				// Download missing content
				UIView->RunContentDownloader();
			}
		}
	}

	s_firstFrame = false;

	// Check resolution
	/*RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	if(desktopRect.right *RES_UPSCALE != GetResolution().x || desktopRect.bottom * RES_UPSCALE != GetResolution().y)
	{
		// Match the resolution with the current desktop resolution
		OnResize(INT2(desktopRect.right * RES_UPSCALE, desktopRect.bottom * RES_UPSCALE));
	}*/

	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	ID3D11CommandList* dc_cl = NULL;
	DeferredContext->FinishCommandList(true, &dc_cl);

	// Copy list of textures we are operating on
	Engine::GAPI->MoveLoadedTexturesToProcessedList();
	Engine::GAPI->ClearFrameLoadedTextures();

	Engine::GAPI->LeaveResourceCriticalSection();
	if (dc_cl)
	{
		//LogInfo() << "Executing command list";
		Context->ExecuteCommandList(dc_cl, true);
		dc_cl->Release();
	}

	// Check for editorpanel
	if(!UIView)
	{
		if(Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel)
		{
			CreateMainUIView();
		}
	}

	// Check for shadowmap resize
	int s = Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize;
	float r = 0;
	switch(s)
	{
	case 0:
		s = 512;
		break;

	case 1:
		s = 1024;
		break;

	case 2:
		s = 2048;
		break;

	case 3:
		s = 4096;
		break;
	}

	if(WorldShadowmap1->GetSizeX() != s)
	{
		int old = WorldShadowmap1->GetSizeX();
		LogInfo() << "Shadowmapresolution changed to: " << s << "x" << s;
		delete WorldShadowmap1; WorldShadowmap1 = NULL;
		WorldShadowmap1 = new RenderToDepthStencilBuffer(Device, s, s, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

		Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale *= old / (float)s;
	}

	// Force the mode
	zCView::SetMode((int)(Resolution.x / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), (int)(Resolution.y / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), 32);

	//return XR_SUCCESS;

	// Notify the shader manager
	ShaderManager->OnFrameStart();

	// Clear using the fog color
	//Clear(float4(0,0,0,0));
	
	// Enable blending, in case gothic want's to render its save-screen
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	UpdateRenderStates();

	// Bind GBuffers
	//ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	//Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	PS_DiffuseNormalmappedFxMap = ShaderManager->GetPShader("PS_DiffuseNormalmappedFxMap");
	PS_DiffuseNormalmappedAlphatestFxMap = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphatestFxMap");
	PS_DiffuseNormalmapped = ShaderManager->GetPShader("PS_DiffuseNormalmapped");
	PS_Diffuse = ShaderManager->GetPShader("PS_Diffuse");
	PS_DiffuseNormalmappedAlphatest = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphaTest");
	PS_DiffuseAlphatest = ShaderManager->GetPShader("PS_DiffuseAlphaTest");
	PS_Simple = ShaderManager->GetPShader("PS_Simple");
	GS_Billboard = ShaderManager->GetGShader("GS_Billboard");
	PS_LinDepth = ShaderManager->GetPShader("PS_LinDepth");
	return XR_SUCCESS;
}

/** Called when the game ended it's frame */
XRESULT D3D11GraphicsEngine::OnEndFrame()
{
	Present();

	// At least Present should have flushed the pipeline, so these textures should be ready by now
	Engine::GAPI->SetFrameProcessedTexturesReady();

	Engine::GAPI->GetRendererState()->RendererInfo.Timing.StopTotal();

	return XR_SUCCESS;
}

/** Called when the game wants to clear the bound rendertarget */
XRESULT D3D11GraphicsEngine::Clear(const float4& color)
{


	//Context->ClearRenderTargetView(BackbufferRTV, (float *)&D3DXVECTOR4(1,0,0,0));
	Context->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	Context->ClearRenderTargetView(GBuffer0_Diffuse->GetRenderTargetView(), (float *)&color);
	Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	Context->ClearRenderTargetView(HDRBackBuffer->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	
	return XR_SUCCESS;
}

/** Creates a vertexbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateVertexBuffer(BaseVertexBuffer** outBuffer)
{
	*outBuffer = new D3D11VertexBuffer;
	return XR_SUCCESS;
}

/** Creates a texture object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateTexture(BaseTexture** outTexture)
{
	*outTexture = new D3D11Texture;
	return XR_SUCCESS;
}

/** Creates a constantbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateConstantBuffer(BaseConstantBuffer** outCB, void* data, int size)
{
	*outCB = new D3D11ConstantBuffer(size, data);
	return XR_SUCCESS;
}

/** Returns a list of available display modes */
XRESULT D3D11GraphicsEngine::GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling )
{
	HRESULT hr;
	UINT numModes = 0;
	DXGI_MODE_DESC* displayModes = NULL;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	IDXGIOutput* output;

	// Get desktop rect
	RECT desktop;
	GetClientRect(GetDesktopWindow(), &desktop);

	if(!DXGIAdapter)
		return XR_FAILED;

	DXGIAdapter->EnumOutputs(0, &output);

	if(!output)
		return XR_FAILED;

	hr = output->GetDisplayModeList(format, 0, &numModes, NULL);

	displayModes = new DXGI_MODE_DESC[numModes];

	// Get the list
	hr = output->GetDisplayModeList(format, 0, &numModes, displayModes);

	for (unsigned int i = 0; i<numModes; i++)
	{
		if (displayModes[i].Format != format)
			continue;

		DisplayModeInfo info;
		info.Height = displayModes[i].Height;
		info.Width = displayModes[i].Width;
		info.Bpp = 32;

		if(info.Width > (unsigned long)desktop.right || info.Height > (unsigned long)desktop.bottom)
			continue; // Skip bigger sizes than the desktop rect, because DXGI doesn't like them apparently TODO: Fix this if possible!

		if(!modeList->empty() && memcmp(&modeList->back(), &info, sizeof(info)) == 0)
			continue; // Already have that in list

		modeList->push_back(info);
	}

	if(includeSuperSampling)
	{
		// Put supersampling resolutions in, up to just below 8k
		int i = 2;
		DisplayModeInfo ssBase = modeList->back();
		while(ssBase.Width * i < 8192 && ssBase.Height * i < 8192)
		{
			DisplayModeInfo info;
			info.Height = ssBase.Height * i;
			info.Width = ssBase.Width * i;
			info.Bpp = 32;

			modeList->push_back(info);
			i++;
		}
	}


	delete[] displayModes;

	output->Release();

	return XR_SUCCESS;
}

/** Presents the current frame to the screen */
XRESULT D3D11GraphicsEngine::Present()
{
	//Context->ClearRenderTargetView(PfxRenderer->GetTempBuffer()->GetRenderTargetView(), (float *)&float4(0,0,0,1));
	/*PfxRenderer->CopyTextureToRTV(BackbufferSRV, PfxRenderer->GetTempBuffer()->GetRenderTargetView());
	PfxRenderer->BlurTexture(PfxRenderer->GetTempBuffer());

	//Clear(float4(0,1,0,0));
	PfxRenderer->CopyTextureToRTV(PfxRenderer->GetTempBuffer()->GetShaderResView(), BackbufferRTV);*/

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 0.0f;
	vp.Width = (float)GetBackbufferResolution().x;
	vp.Height = (float)GetBackbufferResolution().y;

	Context->RSSetViewports(1, &vp);


	// Copy HDR scene to backbuffer
	SetDefaultStates();

	SetActivePixelShader("PS_PFX_GammaCorrectInv");

	ActivePS->Apply();

	GammaCorrectConstantBuffer gcb;
	gcb.G_Gamma = Engine::GAPI->GetGammaValue();
	gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();
	gcb.G_TextureSize = GetResolution();
	gcb.G_SharpenStrength = Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor;

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), BackbufferRTV, INT2(0,0), true);

	//Context->ClearState();

	Context->OMSetRenderTargets(1, &BackbufferRTV, NULL);

	// Check for movie-frame
	if(Engine::GAPI->GetPendingMovieFrame())
	{
		Engine::GAPI->GetPendingMovieFrame()->BindToPixelShader(0);
		DrawQuad(INT2(0,0), GetBackbufferResolution());

		Engine::GAPI->SetPendingMovieFrame(NULL);
	}

	// Draw ant tweak bar
	SetDefaultStates();
	Engine::AntTweakBar->Draw();
	SetDefaultStates();
	//LineRenderer->ClearCache();

	//if(DebugPointlight)
	//	DebugPointlight->DebugDrawCubeMap();


	if(UIView)UIView->Render(Engine::GAPI->GetFrameTimeSec());

	//Engine::GAPI->EnterResourceCriticalSection();
	bool vsync = Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync;

	Engine::GAPI->EnterResourceCriticalSection();
	if(SwapChain->Present(vsync ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
	{
		switch(Device->GetDeviceRemovedReason())
		{
		case DXGI_ERROR_DEVICE_HUNG:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_HUNG)";
			exit(0);
			break;

		case DXGI_ERROR_DEVICE_REMOVED:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_REMOVED)";
			exit(0);
			break;

		case DXGI_ERROR_DEVICE_RESET:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_RESET)";
			exit(0);
			break;

		case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DRIVER_INTERNAL_ERROR)";
			exit(0);
			break;

		case DXGI_ERROR_INVALID_CALL:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_INVALID_CALL)";
			exit(0);
			break;

		case S_OK:
			LogInfo() << "Device removed, but we're fine!";
			break;

		default:
			LogWarnBox() << "Device Removed! (Unknown reason)";
		}
	}
	Engine::GAPI->LeaveResourceCriticalSection();	

	PresentPending = false;

	return XR_SUCCESS;
}

/** Called to set the current viewport */
XRESULT D3D11GraphicsEngine::SetViewport(const ViewportInfo& viewportInfo)
{
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = (float)viewportInfo.TopLeftX;
	viewport.TopLeftY = (float)viewportInfo.TopLeftY;
	viewport.Width = (float)viewportInfo.Width;
	viewport.Height = (float)viewportInfo.Height;
	viewport.MinDepth = viewportInfo.MinZ;
	viewport.MaxDepth = viewportInfo.MaxZ;

	Context->RSSetViewports(1, &viewport);

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed (World)*/
XRESULT D3D11GraphicsEngine::DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int stride)
{
#ifdef RECORD_LAST_DRAWCALL
	g_LastDrawCall.Type = DrawcallInfo::VB;
	g_LastDrawCall.NumElements = numVertices;
	g_LastDrawCall.BaseVertexLocation = 0;
	g_LastDrawCall.BaseIndexLocation = 0;
	if(!g_LastDrawCall.Check())
		return XR_SUCCESS;
#endif

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, 0);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed (VOBs)*/
XRESULT D3D11GraphicsEngine::DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset)
{
#ifdef RECORD_LAST_DRAWCALL
	g_LastDrawCall.Type = DrawcallInfo::VB_IX;
	g_LastDrawCall.NumElements = numIndices;
	g_LastDrawCall.BaseVertexLocation = 0;
	g_LastDrawCall.BaseIndexLocation = indexOffset;
	if(!g_LastDrawCall.Check())
		return XR_SUCCESS;
#endif

	if(vb)
	{
		UINT offset = 0;
		UINT uStride = sizeof(ExVertexStruct);
		ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
		Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

		if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
		{
			Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
		}else
		{
			Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		}
	}

	if(numIndices)
	{
		//Draw the mesh
		Context->DrawIndexed(numIndices, indexOffset, 0);

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numIndices / 3;
	}
	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::DrawVertexBufferIndexedUINT(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset)
{
#ifdef RECORD_LAST_DRAWCALL
	g_LastDrawCall.Type = DrawcallInfo::VB_IX_UINT;
	g_LastDrawCall.NumElements = numIndices;
	g_LastDrawCall.BaseVertexLocation = 0;
	g_LastDrawCall.BaseIndexLocation = indexOffset;
	if(!g_LastDrawCall.Check())
		return XR_SUCCESS;
#endif

	if(vb)
	{
		UINT offset = 0;
		UINT uStride = sizeof(ExVertexStruct);
		ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
		Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		
	}

	if(numIndices)
	{
		//Draw the mesh
		Context->DrawIndexed(numIndices, indexOffset, 0);

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numIndices / 3;
	}

	return XR_SUCCESS;
}

/** Binds viewport information to the given constantbuffer slot */
XRESULT D3D11GraphicsEngine::BindViewportInformation(const std::string& shader, int slot) 
{
	D3D11_VIEWPORT vp;
	UINT num = 1;
	Context->RSGetViewports(&num, &vp);

	// Update viewport information
	float scale = Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale;
	Temp2Float2[0].x = vp.TopLeftX / scale;
	Temp2Float2[0].y = vp.TopLeftY / scale;
	Temp2Float2[1].x = vp.Width / scale;
	Temp2Float2[1].y = vp.Height / scale;

	D3D11PShader* ps = ShaderManager->GetPShader(shader);
	D3D11VShader* vs = ShaderManager->GetVShader(shader);

	if(vs)
	{
		vs->GetConstantBuffer()[slot]->UpdateBuffer(Temp2Float2);
		vs->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	if(ps)
	{
		ps->GetConstantBuffer()[slot]->UpdateBuffer(Temp2Float2);
		ps->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	return XR_SUCCESS;
}

/** Draws a vertexarray, non-indexed (HUD, 2D)*/
XRESULT D3D11GraphicsEngine::DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	UpdateRenderStates();
	D3D11VShader* vShader = ActiveVS;//ShaderManager->GetVShader("VS_TransformedEx");
	
	// Bind the FF-Info to the first PS slot

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	SetupVS_ExMeshDrawCall();

	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)TempVertexBuffer)->GetVertexBuffer()->GetDesc(&desc);

	if(desc.ByteWidth < stride * numVertices)
	{
		LogInfo() << "TempVertexBuffer too small (" << desc.ByteWidth << "), need " << stride * numVertices << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete TempVertexBuffer;
		TempVertexBuffer = new D3D11VertexBuffer();

		TempVertexBuffer->Init(NULL, stride * numVertices, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	TempVertexBuffer->UpdateBuffer(vertices, stride * numVertices);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = TempVertexBuffer->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, startVertex);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a vertexarray, indexed */
XRESULT D3D11GraphicsEngine::DrawIndexedVertexArray(ExVertexStruct* vertices, unsigned int numVertices, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int stride)
{
	UpdateRenderStates();
	D3D11VShader* vShader = ActiveVS;//ShaderManager->GetVShader("VS_TransformedEx");
	
	// Bind the FF-Info to the first PS slot

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	SetupVS_ExMeshDrawCall();

	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)TempVertexBuffer)->GetVertexBuffer()->GetDesc(&desc);

	if(desc.ByteWidth < stride * numVertices)
	{
		LogInfo() << "TempVertexBuffer too small (" << desc.ByteWidth << "), need " << stride * numVertices << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete TempVertexBuffer;
		TempVertexBuffer = new D3D11VertexBuffer();

		TempVertexBuffer->Init(NULL, stride * numVertices, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	TempVertexBuffer->UpdateBuffer(vertices, stride * numVertices);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffers[] = {TempVertexBuffer->GetVertexBuffer(), ((D3D11VertexBuffer *)ib)->GetVertexBuffer()};
	Context->IASetVertexBuffers(0, 2, buffers, &uStride, &offset);

	//Draw the mesh
	Context->DrawIndexed(numIndices, 0, 0);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}


/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
XRESULT D3D11GraphicsEngine::DrawVertexBufferFF(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	SetupVS_ExMeshDrawCall();

	// Bind the FF-Info to the first PS slot
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, startVertex);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a skeletal mesh */
XRESULT D3D11GraphicsEngine::DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms, float fatness, SkeletalMeshVisualInfo* msh)
{
	Context->RSSetState(WorldRasterizerState);
	Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);

	if(GetRenderingStage() == DES_SHADOWMAP_CUBE)
		SetActiveVertexShader("VS_ExSkeletalCube");
	else 
		SetActiveVertexShader("VS_ExSkeletal");

	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Get currently bound texture name
	zCTexture* tex = Engine::GAPI->GetBoundTexture(0);

	bool tesselationEnabled = Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation;

	if(tex)
	{
		MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom(tex);
		if(!info->Constantbuffer)
			info->UpdateConstantbuffer(); // TODO: Slow, save this somewhere!

		// FIXME: Currently bodies and faces look really glossy. 
		//		  This is only a temporary fix!	
		if(info->buffer.SpecularIntensity != 0.05f)
		{
			info->buffer.SpecularIntensity = 0.05f;
			info->UpdateConstantbuffer(); 
		}

		info->Constantbuffer->BindToPixelShader(2);

		// Bind a default normalmap in case the scene is wet and we currently have none
		if(!tex->GetSurface()->GetNormalmap())
		{
			// Modify the strength of that default normalmap for the material info
			if(info->buffer.NormalmapStrength/* * Engine::GAPI->GetSceneWetness()*/ != DEFAULT_NORMALMAP_STRENGTH)
			{
				info->buffer.NormalmapStrength = DEFAULT_NORMALMAP_STRENGTH;
				info->UpdateConstantbuffer();
			}

			DistortionTexture->BindToPixelShader(1);
		}

		// Select shader
		BindShaderForTexture(tex);
	
		if(RenderingStage == DES_MAIN)
		{
			if(tesselationEnabled && msh && msh->TesselationInfo.buffer.VT_TesselationFactor > 0.0f)
			{
				MyDirectDrawSurface7* surface = tex->GetSurface();
				ID3D11ShaderResourceView* srv = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
				// Set normal/displacement map
				Context->DSSetShaderResources(0,1, &srv);
				Context->HSSetShaderResources(0,1, &srv);
				Setup_PNAEN(PNAEN_Skeletal);
				msh->TesselationInfo.Constantbuffer->BindToDomainShader(1);
				msh->TesselationInfo.Constantbuffer->BindToHullShader(1);
			}else if(ActiveHDS)
			{
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Context->DSSetShader(NULL, NULL, NULL);
				Context->HSSetShader(NULL, NULL, NULL);
				ActiveHDS = NULL;
			}
		}
	}



	VS_ExConstantBuffer_PerInstanceSkeletal cb2;
	cb2.World = world;
	cb2.PI_ModelFatness = fatness;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb2);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Copy bones
	memcpy(TempBonesD3DXmatrix, &transforms[0], sizeof(D3DXMATRIX) * std::min(transforms.size(), sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0])));
	ActiveVS->GetConstantBuffer()[2]->UpdateBuffer(TempBonesD3DXmatrix);
	ActiveVS->GetConstantBuffer()[2]->BindToVertexShader(2);

	if(transforms.size() >= sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0]))
	{
		LogWarn() << "SkeletalMesh has more than " << sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0]) << " bones! (" << transforms.size() << ")Up this limit!";
	}

	ActiveVS->Apply();
	ActivePS->Apply();

	
	UINT offset = 0;
	UINT uStride = sizeof(ExSkelVertexStruct);
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	if(RenderingStage == DES_SHADOWMAP)
	{
		// Unbind PixelShader in this case
		Context->PSSetShader(NULL, NULL, NULL);
		ActivePS = NULL;
	}

	bool linearDepth = (Engine::GAPI->GetRendererState()->GraphicsState.FF_GSwitches & GSWITCH_LINEAR_DEPTH) != 0;
	if(linearDepth)
	{
		ActivePS = PS_LinDepth;
		ActivePS->Apply();
	}

	//Draw the mesh
	Context->DrawIndexed(numIndices, 0, 0);

	if(ActiveHDS)
	{
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->DSSetShader(NULL, NULL, NULL);
		Context->HSSetShader(NULL, NULL, NULL);
		ActiveHDS = NULL;
	}

	return XR_SUCCESS;
}

/** Draws a batch of instanced geometry */
XRESULT D3D11GraphicsEngine::DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride)
{
	UpdateRenderStates();

	// Check buffersize
	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

	if(desc.ByteWidth < instanceDataStride * numInstances)
	{
		LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << instanceDataStride * numInstances << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete DynamicInstancingBuffer;
		DynamicInstancingBuffer = new D3D11VertexBuffer();

		// Put in some little extra space (16) so we don't need to recreate this every frame when approaching a field of stones or something.
		DynamicInstancingBuffer->Init(NULL, instanceDataStride * (numInstances + 16), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	// Update the vertexbuffer
	DynamicInstancingBuffer->UpdateBuffer(instanceData, instanceDataStride * numInstances);

	// Bind shader and pipeline flags
	//Context->RSSetState(WorldRasterizerState);
	//Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	D3D11VShader* vShader = ShaderManager->GetVShader("VS_ExInstanced");

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	
	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;

	VS_ExConstantBuffer_PerInstance cb2;
	cb2.World = world;




	vShader->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	vShader->GetConstantBuffer()[0]->BindToVertexShader(0);

	//vShader->GetConstantBuffer()[1]->UpdateBuffer(&cb2);
	//vShader->GetConstantBuffer()[1]->BindToVertexShader(1);

	vShader->Apply();
	
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset[] = {0,0};
	UINT uStride[] = {vertexStride, instanceDataStride};
	ID3D11Buffer* buffers[] = {((D3D11VertexBuffer *)vb)->GetVertexBuffer(),
		((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()};
	Context->IASetVertexBuffers(0, 2, buffers, uStride, offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	//Draw the batch
	Context->DrawIndexedInstanced(numIndices, numInstances, 0, 0, 0);

	return XR_SUCCESS;
}

/** Draws a batch of instanced geometry */
XRESULT D3D11GraphicsEngine::DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, BaseVertexBuffer* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride, unsigned int startInstanceNum, unsigned int indexOffset)
{
	// Bind shader and pipeline flags
	UINT offset[] = {0,0};
	UINT uStride[] = {vertexStride, instanceDataStride};
	ID3D11Buffer* buffers[] = {((D3D11VertexBuffer *)vb)->GetVertexBuffer(),
		((D3D11VertexBuffer *)instanceData)->GetVertexBuffer()};
	Context->IASetVertexBuffers(0, 2, buffers, uStride, offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	unsigned int max = Engine::GAPI->GetRendererState()->RendererSettings.MaxNumFaces * 3;
	numIndices = max != 0 ? (numIndices < max ? numIndices : max) : numIndices;

	//Draw the batch
	Context->DrawIndexedInstanced(numIndices, numInstances, indexOffset, 0, startInstanceNum);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs++;

	return XR_SUCCESS;
}

/** Sets the active pixel shader object */
XRESULT D3D11GraphicsEngine::SetActivePixelShader(const std::string& shader)
{
	ActivePS = ShaderManager->GetPShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::SetActiveVertexShader(const std::string& shader)
{
	ActiveVS = ShaderManager->GetVShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::SetActiveHDShader(const std::string& shader)
{
	ActiveHDS = ShaderManager->GetHDShader(shader);

	return XR_SUCCESS;
}

/** Binds the active PixelShader */
XRESULT D3D11GraphicsEngine::BindActivePixelShader()
{
	if(ActiveVS)ActiveVS->Apply();
	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::BindActiveVertexShader()
{
	if(ActivePS)ActivePS->Apply();
	return XR_SUCCESS;
}

/** Unbinds the texture at the given slot */
XRESULT D3D11GraphicsEngine::UnbindTexture(int slot)
{
	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(slot, 1, &srv);
	Context->VSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Recreates the renderstates */
XRESULT D3D11GraphicsEngine::UpdateRenderStates()
{
	if (Engine::GAPI->GetRendererState()->BlendState.StateDirty && Engine::GAPI->GetRendererState()->BlendState.Hash != FFBlendStateHash)
	{
		D3D11BlendStateInfo* state = (D3D11BlendStateInfo *)GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState];

		if(!state)
		{
			// Create new state
			state = new D3D11BlendStateInfo(Engine::GAPI->GetRendererState()->BlendState);

			GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState] = state;
		}

		FFBlendState = state->State;
		FFRasterizerStateHash = Engine::GAPI->GetRendererState()->BlendState.Hash;

		Engine::GAPI->GetRendererState()->BlendState.StateDirty = false;
		Context->OMSetBlendState(FFBlendState, (float *)&D3DXVECTOR4(0, 0, 0, 0), 0xFFFFFFFF);
	}

	

	if (Engine::GAPI->GetRendererState()->RasterizerState.StateDirty && Engine::GAPI->GetRendererState()->RasterizerState.Hash != FFRasterizerStateHash)
	{
		D3D11RasterizerStateInfo* state = (D3D11RasterizerStateInfo *)GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState];

		if(!state)
		{
			// Create new state
			state = new D3D11RasterizerStateInfo(Engine::GAPI->GetRendererState()->RasterizerState);

			GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState] = state;
		}

		FFRasterizerState = state->State;
		FFRasterizerStateHash = Engine::GAPI->GetRendererState()->DepthState.Hash;

		Engine::GAPI->GetRendererState()->RasterizerState.StateDirty = false;
		Context->RSSetState(FFRasterizerState);
	}

	

	if (Engine::GAPI->GetRendererState()->DepthState.StateDirty && Engine::GAPI->GetRendererState()->DepthState.Hash != FFDepthStencilStateHash)
	{
		D3D11DepthBufferState* state = (D3D11DepthBufferState *)GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState];

		if(!state)
		{
			// Create new state
			state = new D3D11DepthBufferState(Engine::GAPI->GetRendererState()->DepthState);

			GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState] = state;
		}

		FFDepthStencilState = state->State;
		FFDepthStencilStateHash = Engine::GAPI->GetRendererState()->DepthState.Hash;

		Engine::GAPI->GetRendererState()->DepthState.StateDirty = false;
		Context->OMSetDepthStencilState(FFDepthStencilState, 0);
	}

	

	return XR_SUCCESS;
}

/** Called when we started to render the world */
XRESULT D3D11GraphicsEngine::OnStartWorldRendering()
{
	//Clear(float4(0,0,0,0));
	//Clear(float4(0xFF44AEFF));

	SetDefaultStates();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering)
		return XR_SUCCESS;

	//return XR_SUCCESS;
	if(PresentPending)
		return XR_SUCCESS;

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)GetResolution().x;
	vp.Height = (float)GetResolution().y;

	Context->RSSetViewports(1, &vp);

	ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	Clear(float4(Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor, 0.0f));

	// Clear textures from the last frame
	FrameTextures.clear(); 
	RenderedVobs.resize(0); // Keep memory allocated on this
	FrameWaterSurfaces.clear();
	FrameTransparencyMeshes.clear();

	// FIXME: Hack for texture caching!
	zCTextureCacheHack::NumNotCachedTexturesInFrame = 0;

	// Re-Bind the default sampler-state in case it was overwritten
	Context->PSSetSamplers(0, 1, &DefaultSamplerState);

	// Update view distances
	InfiniteRangeConstantBuffer->UpdateBuffer(&D3DXVECTOR4(FLT_MAX, 0, 0, 0));
	OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius, 0, 0, 0));
	OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius, 0, 0, 0));


	// Update editor
	if(UIView)UIView->Update(Engine::GAPI->GetFrameTimeSec());

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = false;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSky)
	{
		// Draw back of the sky if outdoor
		DrawSky();
	}

	// Draw world
	Engine::GAPI->DrawWorldMeshNaive();

	

	// Draw HBAO
	if(Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Enabled)
		PfxRenderer->DrawHBAO(HDRBackBuffer->GetRenderTargetView());

	SetDefaultStates();
	

	//PfxRenderer->RenderDistanceBlur();
	
	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	// Draw water surfaces of current frame
	DrawWaterSurfaces();

	// Draw light-shafts
	DrawMeshInfoListAlphablended(FrameTransparencyMeshes);

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawFog && 
		Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_OUTDOOR)
		PfxRenderer->RenderHeightfog();

	// Draw rain
	if(Engine::GAPI->GetRainFXWeight() > 0.0f)
		Effects->DrawRain();

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	SetDefaultStates();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	// Draw unlit decals //FIXME: Only get them once!
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawParticleEffects)
	{
		std::vector<zCVob *> decals;
		Engine::GAPI->GetVisibleDecalList(decals);

		// Draw stuff like candle-flames
		DrawDecalList(decals, false);
	}

	// FIXME: GodRays need the GBuffer1 from the scene, but Particles need to clear it! 
	if(Engine::GAPI->GetRendererState()->RendererSettings.EnableGodRays)
		PfxRenderer->RenderGodRays();

	//DrawParticleEffects();
	Engine::GAPI->DrawParticlesSimple();

	// Draw debug lines
	LineRenderer->Flush();	

	if(Engine::GAPI->GetRendererState()->RendererSettings.EnableHDR)
		PfxRenderer->RenderHDR();

	if(Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA)
		PfxRenderer->RenderSMAA();

	//PfxRenderer->CopyTextureToRTV(GBuffer1_Normals_SpecIntens_SpecPower->GetShaderResView(), BackbufferRTV, INT2(Resolution.x / 4, Resolution.y / 4));

	// Disable the depth-buffer
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	PresentPending = true;


	// Set viewport for gothics rendering
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)GetBackbufferResolution().x;
	vp.Height = (float)GetBackbufferResolution().y;

	Context->RSSetViewports(1, &vp);

	// If we currently are underwater, then draw underwater effects
	if(Engine::GAPI->IsUnderWater())
		DrawUnderwaterEffects();

	// Clear here to get a working depthbuffer but no interferences with world geometry for gothic UI-Rendering
	Context->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	SetDefaultStates();

	// Save screenshot if wanted
	if(SaveScreenshotNextFrame)
	{
		SaveScreenshot();
		SaveScreenshotNextFrame = false;
	}

	return XR_SUCCESS;
}


void D3D11GraphicsEngine::SetupVS_ExMeshDrawCall()
{
	UpdateRenderStates();

	if(ActiveVS)ActiveVS->Apply();
	if(ActivePS)ActivePS->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11GraphicsEngine::SetupVS_ExConstantBuffer()
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;
	cb.ViewProj = proj * view;

	ActiveVS->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[0]->BindToVertexShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToDomainShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToHullShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToGeometryShader(0);
}

void D3D11GraphicsEngine::SetupVS_ExPerInstanceConstantBuffer()
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);
}

/** Puts the current world matrix into a CB and binds it to the given slot */
void D3D11GraphicsEngine::SetupPerInstanceConstantBuffer(int slot)
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(slot);
}

bool SectionRenderlistSortCmp(std::pair<float, WorldMeshSectionInfo*>& a, std::pair<float, WorldMeshSectionInfo*>& b)
{
	return a.first < b.first;
}

/** Test draw world */
void D3D11GraphicsEngine::TestDrawWorldMesh()
{
	std::list<WorldMeshSectionInfo*> renderList;

	Engine::GAPI->CollectVisibleSections(renderList);

	DistortionTexture->BindToPixelShader(0);

	
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	for(std::list<WorldMeshSectionInfo*>::iterator itr = renderList.begin(); itr != renderList.end(); itr++)
	{

		for(std::map<BaseTexture *, std::vector<MeshInfo*>>::iterator it = (*itr)->WorldMeshesByCustomTexture.begin(); it != (*itr)->WorldMeshesByCustomTexture.end();it++)
		{
			if((*it).first)
			{
				(*it).first->BindToPixelShader(0);
			}

			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*it).second[i]->Indices.size(), (*it).second[i]->BaseIndexLocation);		
			}
		}

		for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator it = (*itr)->WorldMeshesByCustomTextureOriginal.begin(); it != (*itr)->WorldMeshesByCustomTextureOriginal.end();it++)
		{
			if((*it).first && (*it).first->GetTexture())
			{
				if((*it).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
					(*it).first->GetTexture()->Bind(0);
				else
					continue;
			}

			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*it).second[i]->Indices.size(), (*it).second[i]->BaseIndexLocation);		
			}
		}
	}

	/*int x=0;
	for(stdext::unordered_map<zCTexture*, std::vector<MeshInfo *>>::iterator it = meshesByTexture.begin();it != meshesByTexture.end(); it++)
	{
			
	}*/

}

/** Draws a list of mesh infos */
XRESULT D3D11GraphicsEngine::DrawMeshInfoListAlphablended(const std::vector<std::pair<MeshKey, MeshInfo*>>& list)
{
	SetDefaultStates();

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();


	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);
	Engine::GAPI->ResetWorldTransform();

	SetActivePixelShader("PS_Diffuse");
	SetActiveVertexShader("VS_Ex");

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	// Bind wrapped mesh vertex buffers
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	int lastAlphaFunc = 0;

	// Draw the list
	for(auto it = list.begin(); it != list.end(); it++)
	{
		int indicesNumMod = 1;
		if((*it).first.Material->GetAniTexture() != NULL)
		{
			MyDirectDrawSurface7* surface = (*it).first.Material->GetAniTexture()->GetSurface();
			ID3D11ShaderResourceView* srv[3];
			
			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;


			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			// Get the right shader for it

			BindShaderForTexture((*it).first.Material->GetAniTexture(), false, (*it).first.Material->GetAlphaFunc());
			
			// Check for alphablending on world mesh
			if(lastAlphaFunc != (*it).first.Material->GetAlphaFunc())
			{
				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
				lastAlphaFunc = (*it).first.Material->GetAlphaFunc();
			}


			MaterialInfo* info = (*it).first.Info;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			info->Constantbuffer->BindToPixelShader(2);

			// Don't let the game unload the texture after some time
			(*it).first.Material->GetAniTexture()->CacheIn(0.6f);

			// Draw the section-part
			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}
	}

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = false;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	UpdateRenderStates();

	// Draw again, but only to depthbuffer this time to make them work with fogging
	for(auto it = list.begin(); it != list.end(); it++)
	{
		if((*it).first.Material->GetAniTexture() != NULL)
		{
			// Draw the section-part
			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}
	}

	SetDefaultStates();

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::DrawWorldMesh(bool noTextures)
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		return XR_SUCCESS;

	SetDefaultStates();

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();


	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);
	Engine::GAPI->ResetWorldTransform();

	SetActivePixelShader("PS_Diffuse");
	SetActiveVertexShader("VS_Ex");

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Bind reflection-cube to slot 4
	Context->PSSetShaderResources(4, 1, &ReflectionCube);

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	std::list<WorldMeshSectionInfo*> renderList;
	Engine::GAPI->CollectVisibleSections(renderList);

	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, 
				Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	std::list<std::pair<MeshKey, WorldMeshInfo *>> meshList;

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->DSSetShader(NULL, NULL, NULL);
	Context->HSSetShader(NULL, NULL, NULL);

	int numUncachedTextures = 0;
	for(int i=0;i<2;i++)
	{
		for(std::list<WorldMeshSectionInfo*>::iterator it = renderList.begin(); it != renderList.end(); it++)
		{
			for(std::map<MeshKey, WorldMeshInfo*>::iterator itm = (*it)->WorldMeshes.begin(); itm != (*it)->WorldMeshes.end();itm++)
			{
				if((*itm).first.Material)
				{
					zCTexture* aniTex = (*itm).first.Material->GetTexture();

					if(!aniTex)
						continue;

					//if(stricmp(aniTex->GetNameWithoutExt().c_str(), "NW_MISC_ROOF_01") == 0) 
					//	LogInfo() << "XXX";

					if(i == 1) // Second try, this is only true if we have many unloaded textures
					{
						aniTex->CacheIn(-1);
					}else
					{
						if(aniTex->CacheIn(0.6f) != zRES_CACHED_IN)
						{
							numUncachedTextures++;
							continue;
						}
					}

					// Check surface type
					if((*itm).first.Info->MaterialType == MaterialInfo::MT_Water)
					{
						FrameWaterSurfaces[aniTex].push_back((*itm).second);
						continue;
					}

					// Check if the animated texture and the registered textures are the same
					if((*itm).first.Texture != aniTex)
					{
						MeshKey key = (*itm).first;
						key.Texture = aniTex;

						// Check for alphablending
						if((*itm).first.Material->GetAlphaFunc() > zMAT_ALPHA_FUNC_FUNC_NONE && (*itm).first.Material->GetAlphaFunc() != zMAT_ALPHA_FUNC_TEST)
						{
							FrameTransparencyMeshes.push_back((*itm));
						}else
						{
							// Create a new pair using the animated texture
							meshList.push_back(std::make_pair(key, (*itm).second));
						}
						
					}else
					{
						// Check for alphablending
						if((*itm).first.Material->GetAlphaFunc() > zMAT_ALPHA_FUNC_FUNC_NONE && (*itm).first.Material->GetAlphaFunc() != zMAT_ALPHA_FUNC_TEST)
						{
							FrameTransparencyMeshes.push_back((*itm));
						}else
						{
							// Push this texture/mesh combination
							meshList.push_back((*itm));
						}				
					}			
				}
			}
		}

		//if(numUncachedTextures < NUM_UNLOADEDTEXCOUNT_FORCE_LOAD_TEXTURES)
		break;

		// If we get here, there are many unloaded textures.
		// Clear the list and try again, with forcing the textures to load
		meshList.clear();
	}

	struct cmpstruct
	{
		static bool cmp(const std::pair<MeshKey, MeshInfo *>& a, const std::pair<MeshKey, MeshInfo *>& b)
		{
			return a.first.Texture < b.first.Texture;
		}
	};

	// Sort by texture
	meshList.sort(cmpstruct::cmp);

	// Draw depth only
	if(Engine::GAPI->GetRendererState()->RendererSettings.DoZPrepass)
	{
		INT2 camSection = WorldConverter::GetSectionOfPos(Engine::GAPI->GetCameraPosition());
		Context->PSSetShader(NULL, NULL, NULL);

		for(std::list<std::pair<MeshKey, WorldMeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
		{
			if(!(*it).first.Material->GetAniTexture())
				continue;

			if((*it).first.Material->GetAniTexture()->HasAlphaChannel())
				continue; // Don't pre-render stuff with alpha channel

			if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
				continue; // Don't pre-render water

			if((*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
				continue; // Don't pre-render tesselated surfaces

			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}
	}

	SetActivePixelShader("PS_Diffuse");
	ActivePS->Apply();

	bool tesselationEnabled = Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation
		&& Engine::GAPI->GetRendererState()->RendererSettings.AllowWorldMeshTesselation;

	// Now draw the actual pixels
	zCTexture* bound = NULL;
	MaterialInfo* boundInfo = NULL;
	ID3D11ShaderResourceView* boundNormalmap = NULL;
	for(std::list<std::pair<MeshKey, WorldMeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
	{
		int indicesNumMod = 1;
		if((*it).first.Texture != bound && Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh > 1)
		{
			MyDirectDrawSurface7* surface = (*it).first.Texture->GetSurface();
			ID3D11ShaderResourceView* srv[3];
			MaterialInfo* info = (*it).first.Info;
			
			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

			// Bind a default normalmap in case the scene is wet and we currently have none
			if(/*Engine::GAPI->GetSceneWetness() > 0.0f && */!srv[1])
			{
				// Modify the strength of that default normalmap for the material info
				if(info->buffer.NormalmapStrength/* * Engine::GAPI->GetSceneWetness()*/ != DEFAULT_NORMALMAP_STRENGTH)
				{
					info->buffer.NormalmapStrength = DEFAULT_NORMALMAP_STRENGTH;
					info->UpdateConstantbuffer();
				}
				srv[1] = ((D3D11Texture*)DistortionTexture)->GetShaderResourceView();
			}

			boundNormalmap = srv[1];

			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			// Get the right shader for it

			BindShaderForTexture((*it).first.Material->GetAniTexture(), false, (*it).first.Material->GetAlphaFunc());
			
			// No backfaceculling for alphatested materials, fixes forests looking stupid in G1
			/*if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_TEST || (*it).first.Texture->HasAlphaChannel())
			{
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS_EQUAL;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}else
			{
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::DEFAULT_DEPTH_COMP_STATE;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}*/

			// Check for alphablending on world mesh
			if(((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND || (*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD) && !Engine::GAPI->GetRendererState()->BlendState.BlendEnabled)
			{
				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}else if(Engine::GAPI->GetRendererState()->BlendState.BlendEnabled && (*it).first.Material->GetAlphaFunc() != zMAT_ALPHA_FUNC_BLEND)
			{
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}

			
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			info->Constantbuffer->BindToPixelShader(2);

			// Don't let the game unload the texture after some timep
			(*it).first.Material->GetAniTexture()->CacheIn(0.6f);

			boundInfo = info;
			bound = (*it).first.Material->GetAniTexture();

			// Bind normalmap to HDS
			if(!(*it).second->IndicesPNAEN.empty())
			{
				Context->DSSetShaderResources(0,1, &boundNormalmap);
				Context->HSSetShaderResources(0,1, &boundNormalmap);
			}

			/*if((*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
			{
				// Set normal/displacement map
				Context->DSSetShaderResources(0,1, &srv[1]);
				Context->HSSetShaderResources(0,1, &srv[1]);
				Setup_PNAEN(PNAEN_Default);

				(*it).second->TesselationSettings.Constantbuffer->BindToDomainShader(1);
				(*it).second->TesselationSettings.Constantbuffer->BindToHullShader(1);
			}else if(ActiveHDS)
			{
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Context->DSSetShader(NULL, NULL, NULL);
				Context->HSSetShader(NULL, NULL, NULL);
				ActiveHDS = NULL;
				SetActiveVertexShader("VS_Ex");
				ActiveVS->Apply();
			}*/
		}

		// Check for tesselated mesh
		//if(tesselationEnabled && !ActiveHDS && (*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
		if(tesselationEnabled && !ActiveHDS && boundInfo->TextureTesselationSettings.buffer.VT_TesselationFactor > 0.0f)
		{
			// Set normal/displacement map
			Context->DSSetShaderResources(0,1, &boundNormalmap);
			Context->HSSetShaderResources(0,1, &boundNormalmap);
			Setup_PNAEN(PNAEN_Default);			
		}

		// Bind infos for this mesh
		if(boundInfo && boundInfo->TextureTesselationSettings.buffer.VT_TesselationFactor > 0.0f 
			&& !(*it).second->IndicesPNAEN.empty() 
			&& (*it).first.Material->GetAlphaFunc() <= zMAT_ALPHA_FUNC_FUNC_NONE && !bound->HasAlphaChannel()) // Only allow tesselation for materials without alphablending
		{
			//(*it).second->TesselationSettings.Constantbuffer->BindToDomainShader(1);
			//(*it).second->TesselationSettings.Constantbuffer->BindToHullShader(1);

			boundInfo->TextureTesselationSettings.Constantbuffer->BindToDomainShader(1);
			boundInfo->TextureTesselationSettings.Constantbuffer->BindToHullShader(1);
		}else if(ActiveHDS) // Unbind tesselation-shaders if the mesh doesn't support it
		{
			Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Context->DSSetShader(NULL, NULL, NULL);
			Context->HSSetShader(NULL, NULL, NULL);
			ActiveHDS = NULL;
			SetActiveVertexShader("VS_Ex");
			ActiveVS->Apply();

			// Bind wrapped mesh again
			DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, 
				Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);
		}

		if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh > 2)
		{
			if(!ActiveHDS)
			{
				// Draw the section-part
				/*static bool test = false;

				if(!test)
				{
					DrawVertexBufferIndexedUINT(NULL, NULL,
						(*it).second->Indices.size(),
						(*it).second->BaseIndexLocation);
				}else*/ // TODO: FIXME!!
				{
					DrawVertexBufferIndexed((*it).second->MeshVertexBuffer, (*it).second->MeshIndexBuffer, (*it).second->Indices.size());
				}
			}
			else
			{
				// Draw from mesh info
				DrawVertexBufferIndexed((*it).second->MeshVertexBuffer, (*it).second->MeshIndexBufferPNAEN, (*it).second->IndicesPNAEN.size());
			}
		}

	}

	SetDefaultStates();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	// FIXME: Remove DrawWorldMeshNaive finally and put this into a proper location!
	UpdateOcclusion();

	return XR_SUCCESS;
}

/** Draws the world mesh */
XRESULT D3D11GraphicsEngine::DrawWorldMeshW(bool noTextures)
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		return XR_SUCCESS;


	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();

	//Engine::GAPI->SetFarPlane(DEFAULT_FAR_PLANE);

	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	

	Engine::GAPI->ResetWorldTransform();
	Engine::GAPI->SetViewTransform(view);

	// Set shader
	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_Ex");
	D3D11VShader* vsEx = ActiveVS;

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

/*#ifdef BUILD_GOTHIC_1_08k
	TestDrawWorldMesh();
	return XR_SUCCESS;
#endif*/

	int numSections = 0;
	int sectionViewDist = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;

	std::list<WorldMeshSectionInfo*> renderList;

	Engine::GAPI->CollectVisibleSections(renderList);

	// Sort the list to help pixel-culling
	//renderList.sort(SectionRenderlistSortCmp);


	//noTextures = true;

	// Static, so we can clear the lists but leave the hashmap intact
	static std::unordered_map<zCTexture*, std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>> meshesByMaterial;
	static zCMesh* startMesh = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh();

	if(startMesh != Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh())
	{
		meshesByMaterial.clear(); // Happens on world change
		startMesh = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh();
	}

	std::vector<MeshInfo*> WaterSurfaces;

	for(std::list<WorldMeshSectionInfo*>::iterator itr = renderList.begin(); itr != renderList.end(); itr++)
	{
		numSections++;

		//GetLineRenderer()->AddAABBMinMax((*itr)->BoundingBox.Min, (*itr)->BoundingBox.Max);

		// Draw world mesh
		/*DistortionTexture->BindToPixelShader(0);
		if((*itr)->FullStaticMesh)
			Engine::GAPI->DrawMeshInfo(NULL, (*itr)->FullStaticMesh);

		continue;*/

		for(std::map<MeshKey, WorldMeshInfo*>::iterator it = (*itr)->WorldMeshes.begin(); it != (*itr)->WorldMeshes.end();it++)
		{
			if((*it).first.Material)
			{
				std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>& p = meshesByMaterial[(*it).first.Material->GetTexture()];
				p.second.push_back((*it).second);

				if(!p.first)
				{
					p.first = Engine::GAPI->GetMaterialInfoFrom((*it).first.Material->GetTextureSingle());
				}
			}
			else
			{
				meshesByMaterial[NULL].second.push_back((*it).second);
				meshesByMaterial[NULL].first = Engine::GAPI->GetMaterialInfoFrom(NULL);
			}
		}
	}

	// Bind wrapped mesh vertex buffers
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	for(std::unordered_map<zCTexture*, std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>>::iterator it = meshesByMaterial.begin(); it != meshesByMaterial.end();it++)
	{
		if((*it).second.second.empty())
			continue;

		if(!(*it).first || !(*it).first)
		{
			DistortionTexture->BindToPixelShader(0);
		}else
		{
			//FrameTextures.insert((*it).first);

			MaterialInfo* info = (*it).second.first;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			// Check surface type
			if(info->MaterialType == MaterialInfo::MT_Water)
			{
				FrameWaterSurfaces[(*it).first] = (*it).second.second;
				(*it).second.second.resize(0);
				continue;
			}

			info->Constantbuffer->BindToPixelShader(2);
			
		
			if((*it).first->GetSurface() && (*it).first->GetSurface()->GetEngineTexture())
			{
				//(*it).first->GetSurface()->GetEngineTexture()->BindToPixelShader(0);
			}else
			{
				//(*it).first->CacheIn(0.6f);
			}
			// Bind texture
			if((*it).first->CacheIn(0.6f) == zRES_CACHED_IN)
				(*it).first->Bind(0);
			else
				continue;

			// TODO: TESTCODE
			/*if(strnicmp((*it).first.Material->GetTexture()->GetNameWithoutExt().c_str(), "NW_Harbour_Stairs", strlen("NW_Harbour_Stairs")) == 0)
			{
			SetActiveVertexShader("VS_ExDisplace");
			ActiveVS->Apply();
			}else
			{
			SetActiveVertexShader("VS_Ex");
			ActiveVS->Apply();
			}*/

			// Querry the second texture slot to see if there is a normalmap bound
			ID3D11ShaderResourceView* nrmmap;
			Context->PSGetShaderResources(1,1, &nrmmap);
			if(!nrmmap)
			{
				if(ActivePS != defaultPS)
				{
					ActivePS = defaultPS;
					ActivePS->Apply();
				}
			}else
			{
				if(ActivePS != nrmPS)
				{
					ActivePS = nrmPS;
					ActivePS->Apply();
				}
				nrmmap->Release();
			}

			// Check for overwrites (TODO: This is slow, sort this!)
			if(!info->VertexShader.empty())
			{
				SetActiveVertexShader(info->VertexShader);
				if(ActiveVS)ActiveVS->Apply();
			}else if(ActiveVS != vsEx)
			{
				ActiveVS = vsEx;
				ActiveVS->Apply();
			}

			if(!info->TesselationShaderPair.empty())
			{
				

				info->Constantbuffer->BindToDomainShader(2);

				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

				D3D11HDShader* hd = ShaderManager->GetHDShader(info->TesselationShaderPair);
				if(hd)hd->Apply();

				ActiveHDS = hd;

				DefaultHullShaderConstantBuffer hscb;

				// convert to EdgesPerScreenHeight
				hscb.H_EdgesPerScreenHeight = GetResolution().y / Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
				hscb.H_Proj11 = Engine::GAPI->GetRendererState()->TransformState.TransformProj._22;
				hscb.H_GlobalTessFactor = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
				hscb.H_ScreenResolution = float2(GetResolution().x, GetResolution().y);
				hscb.H_FarPlane = Engine::GAPI->GetFarPlane();
				hd->GetConstantBuffer()[0]->UpdateBuffer(&hscb);
				hd->GetConstantBuffer()[0]->BindToHullShader(1);
			

			}else if(ActiveHDS)
			{
				ActiveHDS = NULL;
			
				// Bind wrapped mesh vertex buffers
				DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				D3D11HDShader::Unbind();
			}

			if(!info->PixelShader.empty())
			{
				SetActivePixelShader(info->PixelShader);
				if(ActivePS)ActivePS->Apply();

			}else if(ActivePS != defaultPS && ActivePS != nrmPS)
			{
				// Querry the second texture slot to see if there is a normalmap bound
				ID3D11ShaderResourceView* nrmmap;
				Context->PSGetShaderResources(1,1, &nrmmap);
				if(!nrmmap)
				{
					if(ActivePS != defaultPS)
					{
						ActivePS = defaultPS;
						ActivePS->Apply();
					}
				}else
				{
					if(ActivePS != nrmPS)
					{
						ActivePS = nrmPS;
						ActivePS->Apply();
					}
					nrmmap->Release();
				}
			}
		}
		
		if(ActiveHDS)
		{
			for(std::vector<WorldMeshInfo*>::iterator itr = (*it).second.second.begin(); itr != (*it).second.second.end(); itr++)
			{
				DrawVertexBufferIndexed((*itr)->MeshVertexBuffer, (*itr)->MeshIndexBuffer, (*itr)->Indices.size());
			}
		}else
		{
			for(std::vector<WorldMeshInfo*>::iterator itr = (*it).second.second.begin(); itr != (*it).second.second.end(); itr++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*itr)->Indices.size(), (*itr)->BaseIndexLocation);			
			}
		}

		Engine::GAPI->GetRendererState()->RendererInfo.WorldMeshDrawCalls += (*it).second.second.size();

		// Clear the list, leaving the memory allocated
		(*it).second.second.resize(0);
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	Engine::GAPI->GetRendererState()->RendererInfo.FrameNumSectionsDrawn = numSections;
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	return XR_SUCCESS;
}

/** Draws the given mesh infos as water */
void D3D11GraphicsEngine::DrawWaterSurfaces()
{
	SetDefaultStates();

	// Copy backbuffer
	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), PfxRenderer->GetTempBuffer()->GetRenderTargetView());
	CopyDepthStencil();

	// Pre-Draw the surfaces to fix overlaying polygons causing a huge performance drop
	// Unbind pixelshader
	Context->PSSetShader(NULL, NULL, NULL);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());
	for (std::unordered_map<zCTexture*, std::vector<WorldMeshInfo*>>::const_iterator it = FrameWaterSurfaces.begin(); it != FrameWaterSurfaces.end(); it++)
	{
		// Draw surfaces
		for (unsigned int i = 0; i<(*it).second.size(); i++)
		{
			DrawVertexBufferIndexed((*it).second[i]->MeshVertexBuffer, (*it).second[i]->MeshIndexBuffer, (*it).second[i]->Indices.size());
		}
	}

	// Setup depth state so we can't have multiple layers of water
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view); // Update view transform

	// Bind water shader
	SetActiveVertexShader("VS_ExWater");
	SetActivePixelShader("PS_Water");
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();



	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Bind distortion texture
	DistortionTexture->BindToPixelShader(4);

	// Bind copied backbuffer
	Context->PSSetShaderResources(5, 1, PfxRenderer->GetTempBuffer()->GetShaderResViewPtr());

	// Unbind depth buffer
	//Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	// Bind depth to the shader
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	// Fill refraction info CB and bind it
	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	// Bind reflection cube
	Context->PSSetShaderResources(3,1, &ReflectionCube);



	for(int i=0;i<1;i++) // Draw twice, but second time only to depth buffer to fix the fog
	{
		for(std::unordered_map<zCTexture*, std::vector<WorldMeshInfo*>>::const_iterator it = FrameWaterSurfaces.begin(); it != FrameWaterSurfaces.end();it++)
		{
			if((*it).first)
			{
				// Bind diffuse
				if((*it).first->CacheIn(-1) == zRES_CACHED_IN) // Force immediate cache in, because water is important!
					(*it).first->Bind(0);
			}

			// Draw surfaces
			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				DrawVertexBufferIndexed((*it).second[i]->MeshVertexBuffer, (*it).second[i]->MeshIndexBuffer, (*it).second[i]->Indices.size());
			}
		}
	}

	// Draw Ocean
	if(Engine::GAPI->GetOcean())
		Engine::GAPI->GetOcean()->Draw();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS_EQUAL;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
}


/** Draws everything around the given position */
void D3D11GraphicsEngine::DrawWorldAround(const D3DXVECTOR3& position, 
										  float range, 
										  bool cullFront, 
										  bool indoor,
										  bool noNPCs,
										  std::list<VobInfo*>* renderedVobs, 
										  std::list<SkeletalVobInfo*>* renderedMobs,
										  std::map<MeshKey, WorldMeshInfo*, cmpMeshKey>* worldMeshCache)
{
	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = cullFront ? GothicRasterizerStateInfo::CM_CULL_FRONT : GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	bool linearDepth = (Engine::GAPI->GetRendererState()->GraphicsState.FF_GSwitches & GSWITCH_LINEAR_DEPTH)!= 0;
	if(linearDepth)
	{
		SetActivePixelShader("PS_LinDepth");
	}

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Update and bind buffer of PS
	PerObjectState ocb;
	ocb.OS_AmbientColor = float3(1,1,1);
	ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&ocb);
	ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);

	INT2 s = WorldConverter::GetSectionOfPos(position);
	D3DXVECTOR2 camXZ = D3DXVECTOR2(position.x, position.z);

	float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;
	float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;

	DistortionTexture->BindToPixelShader(0);

	// Unbind PS
	//Context->PSSetShader(NULL, NULL, NULL);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	UpdateRenderStates();

	bool colorWritesEnabled = Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled;
	float alphaRef = Engine::GAPI->GetRendererState()->GraphicsState.FF_AlphaRef;

	std::vector<WorldMeshSectionInfo*> drawnSections;

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
	{
		// Bind wrapped mesh vertex buffers
		DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

		D3DXMATRIX id;
		D3DXMatrixIdentity(&id);
		ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
		ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

		// Only use cache if we haven't already collected the vobs
		// TODO: Collect vobs in a different way than using the drawn sections!
		//		 The current solution won't use the cache at all when there are no vobs near!
		if(worldMeshCache && renderedVobs && !renderedVobs->empty())
		{
			for(std::map<MeshKey, WorldMeshInfo*>::iterator it = worldMeshCache->begin(); it != worldMeshCache->end();it++)
			{
				// Bind texture			
				if((*it).first.Material && (*it).first.Material->GetTexture())
				{
					// Check surface type
					if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
					{
						continue;
					}

					if((*it).first.Material->GetTexture()->HasAlphaChannel() || colorWritesEnabled)
					{
						if(alphaRef > 0.0f && (*it).first.Material->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							(*it).first.Material->GetTexture()->Bind(0);
							ActivePS->Apply();
						}else
							continue; // Don't render if not loaded
					}else
					{
						if(!linearDepth) // Only unbind when not rendering linear depth
						{
							// Unbind PS
							Context->PSSetShader(NULL, NULL, NULL);
						}
					}
				}

				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, (*it).second->Indices.size(), (*it).second->BaseIndexLocation);
			}

		}else
		{
			for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = Engine::GAPI->GetWorldSections().begin(); itx != Engine::GAPI->GetWorldSections().end(); itx++)
			{
				for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
				{
					D3DXVECTOR2 a = D3DXVECTOR2((float)((*itx).first - s.x), (float)((*ity).first - s.y));
					if(D3DXVec2Length(&a) < 2)
					{
						WorldMeshSectionInfo& section = (*ity).second;
						drawnSections.push_back(&section);

						if(Engine::GAPI->GetRendererState()->RendererSettings.FastShadows)
						{
							// Draw world mesh
							if(section.FullStaticMesh)
								Engine::GAPI->DrawMeshInfo(NULL, section.FullStaticMesh);
						}else
						{
							for(std::map<MeshKey, WorldMeshInfo*>::iterator it = section.WorldMeshes.begin(); it != section.WorldMeshes.end();it++)
							{
								// Check surface type
								if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
								{
									continue;
								}

								// Bind texture			
								if((*it).first.Material && (*it).first.Material->GetTexture())
								{
									if((*it).first.Material->GetTexture()->HasAlphaChannel() || colorWritesEnabled)
									{
										if(alphaRef > 0.0f && (*it).first.Material->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
										{
											(*it).first.Material->GetTexture()->Bind(0);
											ActivePS->Apply();
										}else
											continue; // Don't render if not loaded
									}else
									{
										if(!linearDepth) // Only unbind when not rendering linear depth
										{
											// Unbind PS
											Context->PSSetShader(NULL, NULL, NULL);
										}
									}
								}

								// Draw from wrapped mesh
								DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, (*it).second->Indices.size(), (*it).second->BaseIndexLocation);

								//Engine::GAPI->DrawMeshInfo((*it).first.Material, (*it).second);
							}
						}
					}
				}
			}
		}
	}


	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		// Draw visible vobs here
		std::list<VobInfo*> rndVob;

		// construct new renderedvob list or fake one
		if(!renderedVobs || renderedVobs->empty())
		{
			for(size_t i=0;i<drawnSections.size();i++)
			{
				for(auto it = drawnSections[i]->Vobs.begin(); it != drawnSections[i]->Vobs.end(); it++)
				{
					if(!(*it)->VisualInfo)
						continue; // Seems to happen in Gothic 1

					if(!(*it)->Vob->GetShowMainVisual())
						continue;

					// Check vob range
					float dist = D3DXVec3Length(&(position - (*it)->LastRenderPosition));
					if(dist > range)
						continue;

					// Check for inside vob. Don't render inside-vobs when the light is outside and vice-versa.
					if(!(*it)->IsIndoorVob && indoor || (*it)->IsIndoorVob && !indoor)
						continue;

					rndVob.push_back((*it));
				}
			}

			if(renderedVobs)
				*renderedVobs = rndVob;
		}

		// At this point eiter renderedVobs or rndVob is filled with something
		std::list<VobInfo*>& rl = renderedVobs != NULL ? *renderedVobs : rndVob;
		for(std::list<VobInfo*>::iterator it = rl.begin(); it != rl.end(); it++)
		{
			

			// Bind per-instance buffer
			((D3D11ConstantBuffer *)(*it)->VobConstantBuffer)->BindToVertexShader(1);

			// Draw the vob
			for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*it)->VisualInfo->Meshes.begin(); itm != (*it)->VisualInfo->Meshes.end();itm++)
			{
				if((*itm).first && (*itm).first->GetTexture())
				{
					if((*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_NONE || 
						(*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT)
					{
						if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							(*itm).first->GetTexture()->Bind(0);
						}
					}else
					{
						DistortionTexture->BindToPixelShader(0);
					}
				}

				for(unsigned int i=0;i<(*itm).second.size();i++)
				{
					Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
				}
			}
		}


		// Vobs have this differently
		Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
		UpdateRenderStates();
	}

	bool renderNPCs = !noNPCs;// && sEngine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows >= GothicRendererSettings::PLS_FULL;
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawMobs)
	{
		// Draw visible vobs here
		std::list<SkeletalVobInfo*> rndVob;

		// construct new renderedvob list or fake one
		if(!renderedMobs || renderedMobs->empty())
		{
			for(std::list<SkeletalVobInfo *>::iterator it = Engine::GAPI->GetSkeletalMeshVobs().begin(); it != Engine::GAPI->GetSkeletalMeshVobs().end(); it++)
			{
				if(!(*it)->VisualInfo)
					continue; // Seems to happen in Gothic 1

				// Check vob range
				float dist = D3DXVec3Length(&(position - (*it)->Vob->GetPositionWorld()));
				if(dist > range)
					continue;

				// Check for inside vob. Don't render inside-vobs when the light is outside and vice-versa.
				if(!(*it)->Vob->IsIndoorVob() && indoor || (*it)->Vob->IsIndoorVob() && !indoor)
					continue;

				// Assume everything that doesn't have a skeletal-mesh won't move very much
				// This applies to usable things like chests, chairs, beds, etc
				if(!((SkeletalMeshVisualInfo *)(*it)->VisualInfo)->SkeletalMeshes.empty())
					continue;

				rndVob.push_back((*it));
			}

			if(renderedMobs)
				*renderedMobs = rndVob;
		}

		// At this point eiter renderedMobs or rndVob is filled with something
		std::list<SkeletalVobInfo*>& rl = renderedMobs != NULL ? *renderedMobs : rndVob;
		for(std::list<SkeletalVobInfo*>::iterator it = rl.begin(); it != rl.end(); it++)
		{
			Engine::GAPI->DrawSkeletalMeshVob((*it), FLT_MAX);
		}

	}
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes)
	{
		// Draw animated skeletal meshes if wanted
		if(renderNPCs)
		{
			for(std::list<SkeletalVobInfo *>::iterator it = Engine::GAPI->GetAnimatedSkeletalMeshVobs().begin(); it != Engine::GAPI->GetAnimatedSkeletalMeshVobs().end(); it++)
			{
				if(!(*it)->VisualInfo)
					continue; // Seems to happen in Gothic 1

				// Check vob range
				float dist = D3DXVec3Length(&(position - (*it)->Vob->GetPositionWorld()));
				if(dist > range)
					continue;

				// Check for inside vob. Don't render inside-vobs when the light is outside and vice-versa.
				if(!(*it)->Vob->IsIndoorVob() && indoor || (*it)->Vob->IsIndoorVob() && !indoor)
					continue;

				Engine::GAPI->DrawSkeletalMeshVob((*it), FLT_MAX);
			}
		}
	}
}

/** Draws everything around the given position */
void D3D11GraphicsEngine::DrawWorldAround(const D3DXVECTOR3& position, int sectionRange, float vobXZRange, bool cullFront, bool dontCull)
{
	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = cullFront ? GothicRasterizerStateInfo::CM_CULL_FRONT : GothicRasterizerStateInfo::CM_CULL_BACK;
	if(dontCull)
		Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;

	Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	Engine::GAPI->ResetWorldTransform();
	Engine::GAPI->SetViewTransform(view);

	// Set shader
	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_DiffuseAlphaTest");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_Ex");

	bool linearDepth = (Engine::GAPI->GetRendererState()->GraphicsState.FF_GSwitches & GSWITCH_LINEAR_DEPTH)!= 0;
	if(linearDepth)
	{
		SetActivePixelShader("PS_LinDepth");
	}

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Update and bind buffer of PS
	PerObjectState ocb;
	ocb.OS_AmbientColor = float3(1,1,1);
	ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&ocb);
	ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);

	INT2 s = WorldConverter::GetSectionOfPos(position);
	D3DXVECTOR2 camXZ = D3DXVECTOR2(position.x, position.z);

	float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;
	float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;

	DistortionTexture->BindToPixelShader(0);

	// Unbind PS
	//Context->PSSetShader(NULL, NULL, NULL);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	UpdateRenderStates();

	bool colorWritesEnabled = Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled;
	float alphaRef = Engine::GAPI->GetRendererState()->GraphicsState.FF_AlphaRef;

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
	{
		// Bind wrapped mesh vertex buffers
		DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

		D3DXMATRIX id;
		D3DXMatrixIdentity(&id);
		ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
		ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

		for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = Engine::GAPI->GetWorldSections().begin(); itx != Engine::GAPI->GetWorldSections().end(); itx++)
		{
			for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
			{
				D3DXVECTOR2 a = D3DXVECTOR2((float)((*itx).first - s.x), (float)((*ity).first - s.y));
				if(D3DXVec2Length(&a) < sectionRange)
				{
					WorldMeshSectionInfo& section = (*ity).second;

					if(Engine::GAPI->GetRendererState()->RendererSettings.FastShadows)
					{
						// Draw world mesh
						if(section.FullStaticMesh)
							Engine::GAPI->DrawMeshInfo(NULL, section.FullStaticMesh);
					}else
					{
						for(std::map<MeshKey, WorldMeshInfo*>::iterator it = section.WorldMeshes.begin(); it != section.WorldMeshes.end();it++)
						{
							// Check surface type
							if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
							{
								continue;
							}

							// Bind texture			
							if((*it).first.Material && (*it).first.Material->GetTexture())
							{
								if((*it).first.Material->GetTexture()->HasAlphaChannel() || colorWritesEnabled)
								{
									if(alphaRef > 0.0f && (*it).first.Material->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
									{
										(*it).first.Material->GetTexture()->Bind(0);
										ActivePS->Apply();
									}else
										continue; // Don't render if not loaded
								}else
								{
									if(!linearDepth) // Only unbind when not rendering linear depth
									{
										// Unbind PS
										Context->PSSetShader(NULL, NULL, NULL);
									}
								}
							}

							// Draw from wrapped mesh
							DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, (*it).second->Indices.size(), (*it).second->BaseIndexLocation);
						
							//Engine::GAPI->DrawMeshInfo((*it).first.Material, (*it).second);
						}

						// Draw all vobs here
						/*for(std::list<VobInfo*>::iterator it = section.Vobs.begin(); it != section.Vobs.end(); it++)
						{
							D3DXVECTOR2 xz = D3DXVECTOR2((*it)->LastRenderPosition.x, (*it)->LastRenderPosition.z);

							if(!(*it)->VisualInfo)
								continue; // Seems to happen in Gothic 1

							// Check vob range
							float range = D3DXVec2Length(&(camXZ - xz));
							if(range > vobOutdoorDist || ((*it)->VisualInfo->MeshSize < vobSmallSize && range > vobOutdoorSmallDist))
								continue;

							// Check for inside vob
							if((*it)->IsIndoorVob)
								continue;

							// Bind per-instance buffer
							((D3D11ConstantBuffer *)(*it)->VobConstantBuffer)->BindToVertexShader(1);

							// Draw the vob
							for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*it)->VisualInfo->Meshes.begin(); itm != (*it)->VisualInfo->Meshes.end();itm++)
							{
								if((*itm).first && (*itm).first->GetTexture())
								{
									if((*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_NONE || 
										(*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT)
									{
										if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
										{
											(*itm).first->GetTexture()->Bind(0);
										}
									}else
									{
										DistortionTexture->BindToPixelShader(0);
									}
								}

								for(unsigned int i=0;i<(*itm).second.size();i++)
								{
									Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
								}
							}
						}*/
					}
				}
			}
		}
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		// Draw visible vobs here
		/*for(std::list<VobInfo*>::iterator it = RenderedVobs.begin(); it != RenderedVobs.end(); it++)
		{
			//D3DXVECTOR2 xz = D3DXVECTOR2((*it)->LastRenderPosition.x, (*it)->LastRenderPosition.z);

			if(!(*it)->VisualInfo)
				continue; // Seems to happen in Gothic 1

			// Check vob range
			float range = D3DXVec3Length(&(position - (*it)->LastRenderPosition));
			if(range > vobOutdoorDist || ((*it)->VisualInfo->MeshSize < vobSmallSize && range > vobOutdoorSmallDist))
				continue;

			// Check for inside vob
			if((*it)->IsIndoorVob)
				continue;

			// Bind per-instance buffer
			((D3D11ConstantBuffer *)(*it)->VobConstantBuffer)->BindToVertexShader(1);

			// Draw the vob
			for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*it)->VisualInfo->Meshes.begin(); itm != (*it)->VisualInfo->Meshes.end();itm++)
			{
				if((*itm).first && (*itm).first->GetTexture())
				{
					if((*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_NONE || 
						(*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT)
					{
						if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							(*itm).first->GetTexture()->Bind(0);
						}
					}else
					{
						DistortionTexture->BindToPixelShader(0);
					}
				}

				for(unsigned int i=0;i<(*itm).second.size();i++)
				{
					Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
				}
				}
				}*/

		// Vobs have this differently
		Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
		UpdateRenderStates();

		// Reset instances
		const std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();
		/*for(std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
		(*it).second->StartNewFrame();
		}*/

		D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
		float shadowRange = Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale * WorldShadowmap1->GetSizeX();
		for(std::list<VobInfo*>::iterator it = RenderedVobs.begin(); it != RenderedVobs.end(); it++)
		{
			VobInstanceInfo vii;
			vii.world = (*it)->WorldMatrix;

			if(!(*it)->IsIndoorVob)// && D3DXVec3Length(&((*it)->LastRenderPosition - position)) < shadowRange)
				((MeshVisualInfo *)(*it)->VisualInfo)->Instances.push_back(vii);
		}

		// Apply instancing shader
		SetActiveVertexShader("VS_ExInstancedObj");
		//SetActivePixelShader("PS_DiffuseAlphaTest");
		ActiveVS->Apply();

		if(!linearDepth) // Only unbind when not rendering linear depth
		{
			// Unbind PS
			Context->PSSetShader(NULL, NULL, NULL);
		}

		D3D11_BUFFER_DESC desc;
		((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

		/*if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
		{
			LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

			// Buffer too small, recreate it
			delete DynamicInstancingBuffer;
			DynamicInstancingBuffer = new D3D11VertexBuffer();
			DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
		}*/


		// Update the vertexbuffer
		//DynamicInstancingBuffer->UpdateBufferAligned16(&s_InstanceData[0], sizeof(VobInstanceInfo) * s_InstanceData.size());

		byte* data;
		UINT size;
		UINT loc = 0;
		DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size);
		static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
		for(std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
			if((*it).second->Instances.empty())
				continue;

			if((loc + (*it).second->Instances.size()) * sizeof(VobInstanceInfo) >= desc.ByteWidth)
				break; // Should never happen

			(*it).second->StartInstanceNum = loc;
			memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());
			loc += (*it).second->Instances.size();
		}
		DynamicInstancingBuffer->Unmap();

		{


			// Draw all vobs the player currently sees
			for(std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
			{
				if((*it).second->Instances.empty())
					continue;

				bool doReset = true;
				for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator itt = (*it).second->MeshesByTexture.begin(); itt != (*it).second->MeshesByTexture.end(); itt++)
				{
					std::vector<MeshInfo *>& mlist = (*it).second->MeshesByTexture[(*itt).first];
					if(mlist.empty())
						continue;

					for(unsigned int i=0;i<mlist.size();i++)
					{
						zCTexture* tx = (*itt).first.Texture;

						// Check for alphablend
						bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
						bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;
						if(!doReset || blendAdd || blendBlend) // FIXME: if one part of the mesh uses blending, all do. 
						{
							doReset = false;
							continue;
						}

						// Bind texture
						if(tx && (tx->HasAlphaChannel() || colorWritesEnabled))
						{
							if(alphaRef > 0.0f && tx->CacheIn(0.6f) == zRES_CACHED_IN)
							{
								tx->Bind(0);
								ActivePS->Apply();
							}else
								continue;
						}else
						{
							if(!linearDepth) // Only unbind when not rendering linear depth
							{
								// Unbind PS
								Context->PSSetShader(NULL, NULL, NULL);
							}
						}

					
						MeshInfo* mi = mlist[i];

						// Draw batch
						DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);

						Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs += (*it).second->Instances.size();
						//Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
					}			
				}

				// Reset visual
				if(doReset)
					(*it).second->StartNewFrame();
			}
		}
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes)
	{
		// Draw skeletal meshes
		for(std::list<SkeletalVobInfo *>::iterator it = Engine::GAPI->GetSkeletalMeshVobs().begin(); it != Engine::GAPI->GetSkeletalMeshVobs().end(); it++)
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABB((*it)->Vob->GetPositionWorld(), D3DXVECTOR3(10,10,10));

			if(!(*it)->VisualInfo)
				continue;

			//if((*it)->VisualInfo->Meshes.empty() && (*it)->VisualInfo->NodeAttachments.empty())
			//	continue;

			INT2 s = WorldConverter::GetSectionOfPos((*it)->Vob->GetPositionWorld());

			float dist = D3DXVec3Length(&((*it)->Vob->GetPositionWorld() - position));
			if(dist > Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius)
				continue; // Skip out of range

			Engine::GAPI->DrawSkeletalMeshVob((*it), FLT_MAX);
		}
	}

	Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
}

/** Draws the static vobs instanced */
XRESULT D3D11GraphicsEngine::DrawVOBsInstanced()
{
	START_TIMING();

	const std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();

	SetDefaultStates();

	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_Diffuse");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_ExInstancedObj");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Use default material info for now
	MaterialInfo defInfo;
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&defInfo);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Vobs need this
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = false;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	bool tesselationEnabled = Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation;

	static std::vector<VobInfo *> vobs;
	static std::vector<VobLightInfo *> lights;
	static std::vector<SkeletalVobInfo *> mobs;
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs || 
		Engine::GAPI->GetRendererState()->RendererSettings.EnableDynamicLighting)
	{
		if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum ||
			(Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum && vobs.empty()))
			Engine::GAPI->CollectVisibleVobs(vobs, lights, mobs);
	}

	// Need to collect alpha-meshes to render them laterdy
	std::list<std::pair<MeshKey, std::pair<MeshVisualInfo*, MeshInfo*>>> AlphaMeshes;
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		// Create instancebuffer for this frame
		D3D11_BUFFER_DESC desc;
		((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

		if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
		{
			LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

			// Buffer too small, recreate it
			delete DynamicInstancingBuffer;
			DynamicInstancingBuffer = new D3D11VertexBuffer();
			DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
		}

		byte* data;
		UINT size;
		UINT loc = 0;
		DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size);
		static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
		for(std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
			(*it).second->StartInstanceNum = loc;
			memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());
			loc += (*it).second->Instances.size();
		}
		DynamicInstancingBuffer->Unmap();

		for(unsigned int i=0;i<vobs.size();i++)
		{
			vobs[i]->VisibleInRenderPass = false; // Reset this for the next frame
			RenderedVobs.push_back(vobs[i]);
		}

		// Reset buffer
		s_InstanceData.resize(0);

		for(std::unordered_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
			if((*it).second->Instances.empty())
				continue;

			if((*it).second->MeshSize < Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize)
			{
				OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
				OutdoorSmallVobsConstantBuffer->BindToPixelShader(3);
			}
			else
			{
				OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
				OutdoorVobsConstantBuffer->BindToPixelShader(3);
			}

			bool doReset = true; // Don't reset alpha-vobs here
			for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator itt = (*it).second->MeshesByTexture.begin(); itt != (*it).second->MeshesByTexture.end(); itt++)
			{
				std::vector<MeshInfo *>& mlist = (*it).second->MeshesByTexture[(*itt).first];
				if(mlist.empty())
					continue;

				// Bind buffers
				DrawVertexBufferIndexed((*it).second->FullMesh->MeshVertexBuffer, (*it).second->FullMesh->MeshIndexBuffer, 0);

				for(unsigned int i=0;i<mlist.size();i++)
				{
					zCTexture* tx = (*itt).first.Material->GetAniTexture();
					MeshInfo* mi = mlist[i];

					if(!tx)
					{
#ifndef BUILD_SPACER
						continue; // Don't render meshes without texture if not in spacer
#else
						// This is most likely some spacer helper-vob
						WhiteTexture->BindToPixelShader(0);
						PS_Diffuse->Apply();

						/*// Apply colors for these meshes
						MaterialInfo::Buffer b;
						ZeroMemory(&b, sizeof(b));
						b.Color = (*itt).first.Material->GetColor();
						PS_Diffuse->GetConstantBuffer()[2]->UpdateBuffer(&b);
						PS_Diffuse->GetConstantBuffer()[2]->BindToPixelShader(2);*/
#endif
					}else
					{
						// Check for alphablending on world mesh
						bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
						bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;
						if(!doReset || blendAdd || blendBlend) // FIXME: if one part of the mesh uses blending, all do. 
						{
							MeshVisualInfo* info = (*it).second;
							MeshInfo* mesh = mlist[i];

							AlphaMeshes.push_back(std::make_pair((*itt).first, std::make_pair(info, mesh)));

							doReset = false;
							continue;
						}

						// Bind texture

						if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							MyDirectDrawSurface7* surface = tx->GetSurface();
							ID3D11ShaderResourceView* srv[3];
							MaterialInfo* info = (*itt).first.Info;
			
							// Get diffuse and normalmap
							srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
							srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
							srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

							// Bind a default normalmap in case the scene is wet and we currently have none
							if(!srv[1])
							{
								// Modify the strength of that default normalmap for the material info
								if(info->buffer.NormalmapStrength/* * Engine::GAPI->GetSceneWetness()*/ != DEFAULT_NORMALMAP_STRENGTH)
								{
									info->buffer.NormalmapStrength = DEFAULT_NORMALMAP_STRENGTH;
									info->UpdateConstantbuffer();
								}
								srv[1] = ((D3D11Texture*)DistortionTexture)->GetShaderResourceView();
							}
							// Bind both
							Context->PSSetShaderResources(0,3, srv);

							// Set normal/displacement map
							Context->DSSetShaderResources(0,1, &srv[1]);
							Context->HSSetShaderResources(0,1, &srv[1]);

							// Force alphatest on vobs for now
							BindShaderForTexture(tx, true, 0);
									
							
							if(!info->Constantbuffer)
								info->UpdateConstantbuffer();

							info->Constantbuffer->BindToPixelShader(2);
						}
						else
						{
#ifndef PUBLIC_RELEASE
							for(size_t s=0;s<(*it).second->Instances.size();s++)
							{
								D3DXVECTOR3 pos = D3DXVECTOR3((*it).second->Instances[s].world._14, (*it).second->Instances[s].world._24, (*it).second->Instances[s].world._34); 
								GetLineRenderer()->AddAABBMinMax(pos - (*it).second->BBox.Min, pos + (*it).second->BBox.Max, D3DXVECTOR4(1,0,0,1));
							}
#endif
							continue;
						}

					}

					if(tesselationEnabled && !mi->IndicesPNAEN.empty() && RenderingStage == DES_MAIN && (*it).second->TesselationInfo.buffer.VT_TesselationFactor > 0.0f)
					{
						
						Setup_PNAEN(PNAEN_Instanced);
						(*it).second->TesselationInfo.Constantbuffer->BindToDomainShader(1);
						(*it).second->TesselationInfo.Constantbuffer->BindToHullShader(1);
					}else if(ActiveHDS)
					{
						Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						Context->DSSetShader(NULL, NULL, NULL);
						Context->HSSetShader(NULL, NULL, NULL);
						ActiveHDS = NULL;
						SetActiveVertexShader("VS_ExInstancedObj");
						ActiveVS->Apply();
					}

					if(!ActiveHDS)
					{
						// Draw batch
						DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);
					}else
					{
						// Draw batch tesselated
						DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBufferPNAEN, mi->IndicesPNAEN.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);
					}
						
						
					//DrawVertexBufferIndexed((*it).second->FullMesh->MeshVertexBuffer, (*it).second->FullMesh->MeshIndexBuffer, 0);

					//Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
				}

			}

			// Reset visual
			if(doReset && !Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
			{
				(*it).second->StartNewFrame();
			}
		}
	}

	// Draw mobs
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawMobs)
	{
		for(unsigned int i=0;i<mobs.size();i++)
		{
			Engine::GAPI->DrawSkeletalMeshVob(mobs[i], FLT_MAX);
			mobs[i]->VisibleInRenderPass = false; // Reset this for the next frame
		}
	}

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->DSSetShader(NULL, NULL, NULL);
	Context->HSSetShader(NULL, NULL, NULL);
	ActiveHDS = NULL;

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	STOP_TIMING(GothicRendererTiming::TT_Vobs);

	
	if(RenderingStage == DES_MAIN)
	{

		if(Engine::GAPI->GetRendererState()->RendererSettings.DrawParticleEffects)
		{
			std::vector<zCVob *> decals;
			Engine::GAPI->GetVisibleDecalList(decals);
			DrawDecalList(decals, true);

			DrawQuadMarks();
		}

		START_TIMING();
		// Draw lighting, since everything is drawn by now and we have the lights here
		DrawLighting(lights);

		STOP_TIMING(GothicRendererTiming::TT_Lighting);
	}

	// Make sure lighting doesn't mess up our state
	SetDefaultStates();

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_ExInstancedObj");

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	for(auto itt = AlphaMeshes.begin(); itt != AlphaMeshes.end(); itt++)
	{
		zCTexture* tx = (*itt).first.Material->GetAniTexture();

		if(!tx)
			continue;

		// Check for alphablending on world mesh
		bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
		bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;

		// Bind texture

		MeshInfo* mi = (*itt).second.second;
		MeshVisualInfo* vi = (*itt).second.first;

		if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
		{
			MyDirectDrawSurface7* surface = tx->GetSurface();
			ID3D11ShaderResourceView* srv[3];

			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			if((blendAdd || blendBlend) && !Engine::GAPI->GetRendererState()->BlendState.BlendEnabled)
			{
				if(blendAdd)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				else if(blendBlend)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}

			MaterialInfo* info = (*itt).first.Info;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();

			info->Constantbuffer->BindToPixelShader(2);
		}

		// Draw batch
		DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), vi->Instances.size(), sizeof(ExVertexStruct), vi->StartInstanceNum);


	}

	// Loop again, now that all alpha-meshes have been rendered
	// so we can reset their visuals, too.
	for(auto itt = AlphaMeshes.begin(); itt != AlphaMeshes.end(); itt++)
	{
		MeshVisualInfo* vi = (*itt).second.first;

		// Reset visual		
		vi->StartNewFrame();	
	}

	if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
	{
		lights.resize(0);
		vobs.resize(0);
		mobs.resize(0);
	}

	SetDefaultStates();

	return XR_SUCCESS;
}

/** Draws the static VOBs */
XRESULT D3D11GraphicsEngine::DrawVOBs(bool noTextures)
{
	return DrawVOBsInstanced();
}


/** Returns the current size of the backbuffer */
INT2 D3D11GraphicsEngine::GetResolution()
{
	return Resolution;
}

/** Returns the actual resolution of the backbuffer (not supersampled) */
INT2 D3D11GraphicsEngine::GetBackbufferResolution()
{
	return Resolution;

	// FIXME: Oversampling
	/*
	// Get desktop rect
	RECT desktop;
	GetClientRect(GetDesktopWindow(), &desktop);

	if(Resolution.x > desktop.right || Resolution.y > desktop.bottom)
		return INT2(desktop.right, desktop.bottom);

	return Resolution;*/
}

/** Sets up the default rendering state */
void D3D11GraphicsEngine::SetDefaultStates(bool force)
{
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->SamplerState.SetDefault();

	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->SamplerState.SetDirty();

	Context->PSSetSamplers(0, 1, &DefaultSamplerState);

	if (force)
	{
		FFRasterizerStateHash = 0;
		FFBlendStateHash = 0;
		FFDepthStencilStateHash = 0;
	}

	UpdateRenderStates();
}

/** Draws the sky using the GSky-Object */
XRESULT D3D11GraphicsEngine::DrawSky()
{
	GSky* sky = Engine::GAPI->GetSky();
	sky->RenderSky();

	//Context->OMSetRenderTargets(1, &BackbufferRTV, NULL);

	if(!Engine::GAPI->GetRendererState()->RendererSettings.AtmosphericScattering)
	{
		Engine::GAPI->GetLoadedWorldInfo()->MainWorld->GetSkyControllerOutdoor()->RenderSkyPre();
		Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);
		return XR_SUCCESS;
	}
	// Create a rotaion only view-matrix
	D3DXMATRIX invView;
	Engine::GAPI->GetInverseViewMatrix(&invView);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, sky->GetAtmoshpereSettings().OuterRadius,
		sky->GetAtmoshpereSettings().OuterRadius,
		sky->GetAtmoshpereSettings().OuterRadius); // Upscale it a huge amount. Gothics world is big.

	D3DXMATRIX world;
	D3DXMatrixTranslation(&world, Engine::GAPI->GetCameraPosition().x, 
		Engine::GAPI->GetCameraPosition().y + sky->GetAtmoshpereSettings().SphereOffsetY, 
		Engine::GAPI->GetCameraPosition().z);
	 
	world = scale * world;
	D3DXMatrixTranspose(&world, &world);

	// Apply world matrix
	Engine::GAPI->SetWorldTransform(world);
	Engine::GAPI->SetViewTransform(view);

	if(sky->GetAtmosphereCB().AC_CameraHeight > sky->GetAtmosphereCB().AC_OuterRadius)
	{
		SetActivePixelShader("PS_AtmosphereOuter");
	}else
	{
		SetActivePixelShader("PS_Atmosphere");
	}

	
	SetActiveVertexShader("VS_ExWS");

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(1);

	VS_ExConstantBuffer_PerInstance cbi;
	cbi.World = world;
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cbi);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);


	// Update sky CB
	/*SkyConstantBuffer scb;
	scb.SC_TextureWeight = factor;
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&scb),
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);
	*/
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();

	// Allow z-testing
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = true;

	// Disable depth-writes so the sky always stays at max distance in the DepthBuffer
	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;

	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Apply sky texture
	BaseTexture* cloudsTex = Engine::GAPI->GetSky()->GetCloudTexture();
	if(cloudsTex)
	{
		cloudsTex->BindToPixelShader(0);
	}

	BaseTexture* nightTex = Engine::GAPI->GetSky()->GetNightTexture();
	if(nightTex)
	{
		nightTex->BindToPixelShader(1);
	}

	

	if(sky->GetSkyDome())
		sky->GetSkyDome()->DrawMesh();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = false;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	//Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	//SetActivePixelShader("PS_World");

	return XR_SUCCESS;
}

/** Called when a key got pressed */
XRESULT D3D11GraphicsEngine::OnKeyDown(unsigned int key)
{
	switch (key)
	{
	case VK_NUMPAD0:
		Engine::GAPI->PrintMessageTimed(INT2(30,30), "Reloading shaders...");
		ReloadShaders();
		break;

	case VK_NUMPAD7:
		SaveScreenshotNextFrame = true;
		break;

	case VK_F1:		
		if(!UIView && !Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel)
		{
			// If the ui-view hasn't been created yet and the editorpanel is disabled, enable it here
			Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel = true;
			CreateMainUIView();
		}
		break;

	default:
		break;
	}

	return XR_SUCCESS;
}

/** Reloads shaders */
XRESULT D3D11GraphicsEngine::ReloadShaders()
{
	XRESULT xr = ShaderManager->ReloadShaders();

	return xr;
}

/** Returns the line renderer object */
BaseLineRenderer* D3D11GraphicsEngine::GetLineRenderer()
{
	return LineRenderer;
}

/** Applys the lighting to the scene */
XRESULT D3D11GraphicsEngine::DrawLighting(std::vector<VobLightInfo*>& lights)
{
	SetDefaultStates();

	// ********************************
	// Draw world shadows
	// ********************************
	CameraReplacement cr;
	D3DXVECTOR3 cameraPosition = Engine::GAPI->GetCameraPosition();
	D3DXVECTOR3 playerPosition = Engine::GAPI->GetPlayerVob() != NULL ? Engine::GAPI->GetPlayerVob()->GetPositionWorld() : D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);

	bool partialShadowUpdate = Engine::GAPI->GetRendererState()->RendererSettings.PartialDynamicShadowUpdates;

	// Draw pointlight shadows
	if(Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows > 0)
	{
		std::list<VobLightInfo*> importantUpdates;
		for(std::vector<VobLightInfo*>::iterator itv = lights.begin(); itv != lights.end();itv++)
		{
			// Create shadowmap in case we should have one but haven't got it yet
			if(!(*itv)->LightShadowBuffers && (*itv)->UpdateShadows)
				Engine::GraphicsEngine->CreateShadowedPointLight(&(*itv)->LightShadowBuffers, (*itv));
		
			if((*itv)->LightShadowBuffers)
			{
				// Check if this lights even needs an update
				bool needsUpdate = ((D3D11PointLight *)(*itv)->LightShadowBuffers)->NeedsUpdate();
				bool wantsUpdate = ((D3D11PointLight *)(*itv)->LightShadowBuffers)->WantsUpdate();

				// Add to the updatequeue if it does
				if(needsUpdate || (*itv)->UpdateShadows)
				{
					// Always update the light if the light itself moved
					if(partialShadowUpdate && !needsUpdate)
					{
						// Only add once. This list should never be very big, so it should be ok to search it like this
						// This needs to be done to make sure a light will get updated only once and won't block the queue
						if(std::find(FrameShadowUpdateLights.begin(), FrameShadowUpdateLights.end(), (*itv)) == FrameShadowUpdateLights.end())
						{
							// Always render the closest light to the playervob, so the player doesn't flicker when moving
							float d = D3DXVec3LengthSq(&((*itv)->Vob->GetPositionWorld() - playerPosition));

							float range = (*itv)->Vob->GetLightRange();
							if(d < range * range && importantUpdates.size() < MAX_IMPORTANT_LIGHT_UPDATES)
							{
								importantUpdates.push_back((*itv));
							}else
							{
								FrameShadowUpdateLights.push_back((*itv));
							}
						}
					}else
					{
						// Always render the closest light to the playervob, so the player doesn't flicker when moving
						float d = D3DXVec3LengthSq(&((*itv)->Vob->GetPositionWorld() - playerPosition));

						float range = (*itv)->Vob->GetLightRange() * 1.5f;

						// If the engine said this light should be updated, then do so. If the light said this
						if(needsUpdate || d < range * range)
							importantUpdates.push_back((*itv));
					}
				}
			}
		}

		//FrameShadowUpdateLights.sort();
		//FrameShadowUpdateLights.unique();

		// Render the closest light
		for(auto it = importantUpdates.begin(); it != importantUpdates.end();it++)
		{
			((D3D11PointLight *)(*it)->LightShadowBuffers)->RenderCubemap((*it)->UpdateShadows);
			(*it)->UpdateShadows = false;
		}
		
		// Update only a fraction of lights, but at least some
		int n = std::max((UINT)NUM_MIN_FRAME_SHADOW_UPDATES, (UINT)(FrameShadowUpdateLights.size() / NUM_FRAME_SHADOW_UPDATES));
		while(!FrameShadowUpdateLights.empty())
		{
			D3D11PointLight* l = (D3D11PointLight *)FrameShadowUpdateLights.front()->LightShadowBuffers;
			
			// Check if we have to force this light to update itself (NPCs moving around, for example)
			bool force = FrameShadowUpdateLights.front()->UpdateShadows;
			FrameShadowUpdateLights.front()->UpdateShadows = false;

			l->RenderCubemap(force);
			DebugPointlight = l;

			FrameShadowUpdateLights.pop_front();

			// Only update n lights
			n--;
			if(n <= 0)
				break;
		}
	}

	// Get shadow direction, but don't update every frame, to get around flickering
	D3DXVECTOR3 dir = *Engine::GAPI->GetSky()->GetAtmosphereCB().AC_LightPos.toD3DXVECTOR3();
	static D3DXVECTOR3 oldDir = dir;
	static D3DXVECTOR3 smoothDir = dir;

	static D3DXVECTOR3 oldP = D3DXVECTOR3(0,0,0);
	
	D3DXVECTOR3 WorldShadowCP;

	// Update dir
	if(fabs(D3DXVec3Dot(&oldDir, &dir)) > 0.9995f)
	{
		dir = oldDir;	
	}
	else
	{
		D3DXVECTOR3 target = dir;

		// Smoothly transition to the next state and wait there
		if(fabs(D3DXVec3Dot(&smoothDir, &dir)) < 0.99995f) // but cut it off somewhere or the pixels will flicker
			D3DXVec3Lerp(&dir, &smoothDir, &target, Engine::GAPI->GetFrameTimeSec() * 2.0f);
		else
			oldDir = dir;	

		smoothDir = dir;
	}

	float smPixelSize = WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale;


	// Update position
	if((D3DXVec3Length(&(oldP - cameraPosition)) < 64 && // Try to update only if the camera went 500 units away from the last position
		(cameraPosition.x - (int)cameraPosition.x) < 0.1f && // And is on even space
		(cameraPosition.y - (int)cameraPosition.y) < 0.1f) || 
		D3DXVec3Length(&(oldP - cameraPosition)) < 600.0f) // but don't let it go too far
	{
		WorldShadowCP = oldP;
	}else
	{
		oldP = cameraPosition;
		WorldShadowCP = cameraPosition;
	}



	// Get the section we are currently in
	INT2 cameraSection = WorldConverter::GetSectionOfPos(WorldShadowCP);
	WorldMeshSectionInfo& section = Engine::GAPI->GetWorldSections()[cameraSection.x][cameraSection.y];
	D3DXVECTOR3 p = WorldShadowCP;
	// Set the camera height to the highest point in this section
	//p.y = 0;
	p += dir * 6000.0f;

	D3DXVECTOR3 lookAt = p;
	lookAt -= dir;

	// Create shadowmap view-matrix
	D3DXMatrixLookAtLH(&cr.ViewReplacement, &p, &lookAt, &D3DXVECTOR3(0,1,0));
	D3DXMatrixTranspose(&cr.ViewReplacement, &cr.ViewReplacement);

	D3DXMatrixOrthoLH(&cr.ProjectionReplacement, WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 
												 WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 1, 20000.0f);
	D3DXMatrixTranspose(&cr.ProjectionReplacement, &cr.ProjectionReplacement);

	cr.PositionReplacement = p;
	cr.LookAtReplacement = lookAt;

	// Replace gothics camera
	Engine::GAPI->SetCameraReplacementPtr(&cr);

	if(Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_OUTDOOR) // Indoor worlds don't need shadowmaps for the world
	{
			RenderShadowmaps(WorldShadowCP, NULL, true);
	}

	SetDefaultStates();

	// Restore gothics camera
	Engine::GAPI->SetCameraReplacementPtr(NULL);

	// Draw rainmap, if raining
	if(Engine::GAPI->GetSceneWetness() > 0.00001f)
	{
		Effects->DrawRainShadowmap();
	}

	

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	// ********************************
	// Draw direct lighting
	// ********************************
	SetActiveVertexShader("VS_ExPointLight");
	SetActivePixelShader("PS_DS_PointLight");

	D3D11PShader* psPointLight = ShaderManager->GetPShader("PS_DS_PointLight");
	D3D11PShader* psPointLightDynShadow = ShaderManager->GetPShader("PS_DS_PointLightDynShadow");

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Copy this, so we can access depth in the pixelshader and still use the buffer for culling
	CopyDepthStencil();

	// Set the main rendertarget
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());
	//Context->ClearRenderTargetView(HDRBackBuffer->GetRenderTargetView(), (float *)D3DXVECTOR4(0,0,0,0));

	
	D3DXMatrixTranspose(&view, &view);

	DS_PointLightConstantBuffer plcb;
	D3DXMatrixInverse(&plcb.PL_InvProj, NULL, &Engine::GAPI->GetProjectionMatrix());
	D3DXMatrixInverse(&plcb.PL_InvView, NULL, &Engine::GAPI->GetRendererState()->TransformState.TransformView);
	plcb.PL_ViewportSize = float2((float)Resolution.x, (float)Resolution.y);

	GBuffer0_Diffuse->BindToPixelShader(Context, 0);
	GBuffer1_Normals_SpecIntens_SpecPower->BindToPixelShader(Context, 1);
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	bool lastOutside = true;

	// Draw all lights
	for(std::vector<VobLightInfo*>::iterator itv = lights.begin(); itv != lights.end();itv++)
	{
		zCVobLight* vob = (*itv)->Vob;

		// Reset state from CollectVisibleVobs
		(*itv)->VisibleInRenderPass = false;

		if(!vob->IsEnabled())
			continue;

		// Set right shader
		if(Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows > 0)
		{
			if((*itv)->LightShadowBuffers && ActivePS != psPointLightDynShadow)
			{
				// Need to update shader for shadowed pointlight
				ActivePS = psPointLightDynShadow;
				ActivePS->Apply();
			}else if(!(*itv)->LightShadowBuffers && ActivePS != psPointLight)
			{
				// Need to update shader for usual pointlight
				ActivePS = psPointLight;
				ActivePS->Apply();
			}
		}

		// Animate the light
		vob->DoAnimation();

		plcb.PL_Color = float4(vob->GetLightColor());
		plcb.PL_Range = vob->GetLightRange();
		plcb.Pl_PositionWorld = vob->GetPositionWorld();
		plcb.PL_Outdoor = (*itv)->IsIndoorVob ? 0.0f : 1.0f;

		float dist = D3DXVec3Length(&(*plcb.Pl_PositionWorld.toD3DXVECTOR3() - Engine::GAPI->GetCameraPosition()));

		// Scale indoor light-distance so they fade out earlier
		//if(vob->IsIndoorVob())
		//	dist /= INDOOR_LIGHT_DISTANCE_SCALE_FACTOR;

		// Gradually fade in the lights
		if( dist + plcb.PL_Range < Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius)
		{
			//float fadeStart = Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius - plcb.PL_Range;
			float fadeEnd = Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius;

			float fadeFactor = std::min(1.0f, std::max(0.0f, ((fadeEnd - (dist + plcb.PL_Range)) / plcb.PL_Range)));
			plcb.PL_Color.x *= fadeFactor;
			plcb.PL_Color.y *= fadeFactor;
			plcb.PL_Color.z *= fadeFactor;
			//plcb.PL_Color.w *= fadeFactor;
		}

		// Make the lights a little bit brighter
		float lightFactor = 1.2f;

		plcb.PL_Color.x *= lightFactor;
		plcb.PL_Color.y *= lightFactor;
		plcb.PL_Color.z *= lightFactor;

		//if(plcb.PL_Range > 10000.0f)
		//	continue;

		//zTBBox3D bb = (*itv)->Vob->GetBBoxLocal();
		//GetLineRenderer()->AddAABBMinMax(bb.Min + (*itv)->Vob->GetPositionWorld() * 0.5f, bb.Max + (*itv)->Vob->GetPositionWorld() * 0.5f, *(D3DXVECTOR4 *)&float4((*itv)->Vob->GetLightColor()));
		//GetLineRenderer()->AddPointLocator(*plcb.Pl_PositionWorld.toD3DXVECTOR3(), 30.0f, *(D3DXVECTOR4 *)&float4((*itv)->Vob->GetLightColor()));

		// Need that in view space
		D3DXVec3TransformCoord(plcb.Pl_PositionView.toD3DXVECTOR3(), plcb.Pl_PositionWorld.toD3DXVECTOR3(), &view);
		D3DXVec3TransformCoord(plcb.PL_LightScreenPos.toD3DXVECTOR3(), plcb.Pl_PositionWorld.toD3DXVECTOR3(), &Engine::GAPI->GetProjectionMatrix());

		if(dist < plcb.PL_Range)
		{
			if(Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled)
			{
				Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled	= false;
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
				UpdateRenderStates();
			}
		}else
		{
			if(!Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled)
			{
				Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled	= true;
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
				UpdateRenderStates();
			}
		}

		plcb.PL_LightScreenPos.x = plcb.PL_LightScreenPos.x/2.0f +0.5f;
		plcb.PL_LightScreenPos.y = plcb.PL_LightScreenPos.y/-2.0f +0.5f;

		// Apply the constantbuffer to vs and PS
		ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&plcb);
		ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);
		ActivePS->GetConstantBuffer()[0]->BindToVertexShader(1); // Bind this instead of the usual per-instance buffer

		if(Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows > 0)
		{
			// Bind shadowmap, if possible
			if((*itv)->LightShadowBuffers)
				((D3D11PointLight *)(*itv)->LightShadowBuffers)->OnRenderLight();
		}

		// Draw the mesh
		InverseUnitSphereMesh->DrawMesh();

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnLights++;
	}


	{
		// *cough cough* somehow this makes the worldmesh disappear in indoor locations
		// TODO: Fixme
		//Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA = false;


		Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

		// Modify light when raining
		float rain = Engine::GAPI->GetRainFXWeight();
		float wetness = Engine::GAPI->GetSceneWetness();

		// Switch global light shader when raining
		if(wetness > 0.0f)
		{
			SetActivePixelShader("PS_DS_AtmosphericScattering_Rain");
		}else
		{
			SetActivePixelShader("PS_DS_AtmosphericScattering");
		}

		//SetActivePixelShader("PS_DS_SimpleSunlight");
		SetActiveVertexShader("VS_PFX");

		SetupVS_ExMeshDrawCall();

		GSky* sky = Engine::GAPI->GetSky();
		ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
		ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

		DS_ScreenQuadConstantBuffer scb;
		scb.SQ_InvProj = plcb.PL_InvProj;
		scb.SQ_InvView = plcb.PL_InvView;
		scb.SQ_View = Engine::GAPI->GetRendererState()->TransformState.TransformView;

		D3DXVec3TransformNormal(scb.SQ_LightDirectionVS.toD3DXVECTOR3(), sky->GetAtmosphereCB().AC_LightPos.toD3DXVECTOR3(), &view);

		float3 sunColor = Engine::GAPI->GetRendererState()->RendererSettings.SunLightColor;

	

		float sunStrength = Toolbox::lerp(Engine::GAPI->GetRendererState()->RendererSettings.SunLightStrength, 
			Engine::GAPI->GetRendererState()->RendererSettings.RainSunLightStrength, 
			std::min(1.0f, rain * 2.0f)); // Scale the darkening-factor faster here, so it matches more with the increasing fog-density



		scb.SQ_LightColor = float4(sunColor.x, sunColor.y, sunColor.z, sunStrength);

		scb.SQ_ShadowView = cr.ViewReplacement;
		scb.SQ_ShadowProj = cr.ProjectionReplacement;
		scb.SQ_ShadowmapSize = (float)WorldShadowmap1->GetSizeX();

		// Get rain matrix
		//scb.SQ_RainViewProj = Effects->GetRainShadowmapCameraRepl().ViewReplacement * Effects->GetRainShadowmapCameraRepl().ProjectionReplacement;
		scb.SQ_RainView = Effects->GetRainShadowmapCameraRepl().ViewReplacement;
		scb.SQ_RainProj = Effects->GetRainShadowmapCameraRepl().ProjectionReplacement;

		//scb.SQ_ProjAB.x = Engine::GAPI->GetFarPlane() / (Engine::GAPI->GetFarPlane() - Engine::GAPI->GetNearPlane());
		//scb.SQ_ProjAB.y = (-Engine::GAPI->GetFarPlane() * Engine::GAPI->GetNearPlane()) / (Engine::GAPI->GetFarPlane() - Engine::GAPI->GetNearPlane());
	
		scb.SQ_ShadowStrength = Engine::GAPI->GetRendererState()->RendererSettings.ShadowStrength;
		scb.SQ_ShadowAOStrength = Engine::GAPI->GetRendererState()->RendererSettings.ShadowAOStrength;
		scb.SQ_WorldAOStrength = Engine::GAPI->GetRendererState()->RendererSettings.WorldAOStrength;

		// Modify lightsettings when indoor
		if(Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_INDOOR)
		{
			// Turn off shadows
			scb.SQ_ShadowStrength = 0.0f;

			// Only use world AO
			scb.SQ_WorldAOStrength = 1.0f;

			// Darken the lights
			scb.SQ_LightColor = float4(1,1,1,DEFAULT_INDOOR_VOB_AMBIENT.x);
		}

		ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&scb);
		ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

		PFXVS_ConstantBuffer vscb;
		vscb.PFXVS_InvProj = scb.SQ_InvProj;
		ActiveVS->GetConstantBuffer()[0]->UpdateBuffer(&vscb);
		ActiveVS->GetConstantBuffer()[0]->BindToVertexShader(0);

		WorldShadowmap1->BindToPixelShader(Context, 3);


		if(Effects->GetRainShadowmap())
			Effects->GetRainShadowmap()->BindToPixelShader(Context, 4);

		Context->PSSetSamplers(2,1, &ShadowmapSamplerState);

		Context->PSSetShaderResources(5, 1, &ReflectionCube2);

		DistortionTexture->BindToPixelShader(6);

		PfxRenderer->DrawFullScreenQuad();

	}

	// Reset state
	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(2, 1, &srv);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());


	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	UpdateRenderStates();


	return XR_SUCCESS;
}


/** Renders the shadowmaps for a pointlight */
void D3D11GraphicsEngine::RenderShadowCube(const D3DXVECTOR3& position, 
										   float range, 
										   RenderToDepthStencilBuffer* targetCube, 
										   ID3D11DepthStencilView* face, 
										   ID3D11RenderTargetView* debugRTV, 
										   bool cullFront,
										   bool indoor,
										   bool noNPCs,
										   std::list<VobInfo*>* renderedVobs, std::list<SkeletalVobInfo*>* renderedMobs, std::map<MeshKey, WorldMeshInfo*, cmpMeshKey>* worldMeshCache)
{
	D3D11_VIEWPORT oldVP;
	UINT n = 1;
	Context->RSGetViewports(&n, &oldVP);

	// Apply new viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)targetCube->GetSizeX();
	vp.Height = (float)targetCube->GetSizeX();
	
	Context->RSSetViewports(1, &vp);

	if(!face)
	{
		// Set cubemap shader
		SetActiveGShader("GS_Cubemap");
		ActiveGS->Apply();
		face = targetCube->GetDepthStencilView();

		SetActiveVertexShader("VS_ExCube");
	}

	// Set the rendering stage
	D3D11ENGINE_RENDER_STAGE oldStage = RenderingStage;
	SetRenderingStage(DES_SHADOWMAP_CUBE);

	// Clear and Bind the shadowmap
	
	//Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&D3DXVECTOR4(1,0,0,0));

	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(3,1,&srv);

	if(!debugRTV)
	{
		Context->OMSetRenderTargets(0, NULL, face);

		Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = true; // Should be false, but needs to be true for SV_Depth to work
		Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	}else
	{
		Context->OMSetRenderTargets(1, &debugRTV, face);

		Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = true;
		Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	}

	//Context->OMSetRenderTargets(1, GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetViewPtr(), WorldShadowmap1->GetDepthStencilView());

	//Engine::GAPI->SetFarPlane(20000.0f);

	// Dont render shadows from the sun when it isn't on the sky
	if (Engine::GAPI->GetRendererState()->RendererSettings.DrawShadowGeometry &&
		Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows)
	{
		Context->ClearDepthStencilView(face, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Draw the world mesh without textures
		DrawWorldAround(position, range, cullFront, indoor, noNPCs, renderedVobs, renderedMobs, worldMeshCache);
	}else
	{
		if(Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection.y <= 0)
		{
			Context->ClearDepthStencilView(face, D3D11_CLEAR_DEPTH, 0.0f, 0); // Always shadow in the night
		}else
		{
			Context->ClearDepthStencilView(face, D3D11_CLEAR_DEPTH, 1.0f, 0); // Clear shadowmap when shadows not enabled
		}
	}

	// Restore state
	SetRenderingStage(oldStage);
	Context->RSSetViewports(1, &oldVP);
	Context->GSSetShader(NULL, NULL, NULL);
	SetActiveVertexShader("VS_Ex");

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	SetRenderingStage(DES_MAIN);
}

/** Renders the shadowmaps for the sun */
void D3D11GraphicsEngine::RenderShadowmaps(const D3DXVECTOR3& cameraPosition, RenderToDepthStencilBuffer* target, bool cullFront, bool dontCull, ID3D11DepthStencilView* dsvOverwrite, ID3D11RenderTargetView* debugRTV)
{
	if(!target)
	{
		target = WorldShadowmap1;
	}

	if(!dsvOverwrite)
		dsvOverwrite = target->GetDepthStencilView();

	D3D11_VIEWPORT oldVP;
	UINT n = 1;
	Context->RSGetViewports(&n, &oldVP);

	// Apply new viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)target->GetSizeX();
	vp.Height = (float)target->GetSizeX();
	Context->RSSetViewports(1, &vp);


	// Set the rendering stage
	D3D11ENGINE_RENDER_STAGE oldStage = RenderingStage;
	SetRenderingStage(DES_SHADOWMAP);

	// Clear and Bind the shadowmap
	
	//Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&D3DXVECTOR4(1,0,0,0));

	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(3,1,&srv);

	if(!debugRTV)
	{
		Context->OMSetRenderTargets(0, NULL, dsvOverwrite);

		Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = false;
		Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	}else
	{
		Context->OMSetRenderTargets(1, &debugRTV, dsvOverwrite);

		Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = true;
		Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	}

	//Context->OMSetRenderTargets(1, GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetViewPtr(), WorldShadowmap1->GetDepthStencilView());

	//Engine::GAPI->SetFarPlane(20000.0f);

	// Dont render shadows from the sun when it isn't on the sky
	if ((target != WorldShadowmap1 || Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection.y > 0) && // Only stop rendering if the sun is down on main-shadowmap //TODO: Take this out of here!
		Engine::GAPI->GetRendererState()->RendererSettings.DrawShadowGeometry &&
		Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows)
	{
		Context->ClearDepthStencilView(dsvOverwrite, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Draw the world mesh without textures
		DrawWorldAround(cameraPosition, 2, 10000.0f, cullFront, dontCull);
	}else
	{
		if(Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection.y <= 0)
		{
			Context->ClearDepthStencilView(dsvOverwrite, D3D11_CLEAR_DEPTH, 0.0f, 0); // Always shadow in the night
		}else
		{
			Context->ClearDepthStencilView(dsvOverwrite, D3D11_CLEAR_DEPTH, 1.0f, 0); // Clear shadowmap when shadows not enabled
		}
	}

	// Restore state
	SetRenderingStage(oldStage);
	Context->RSSetViewports(1, &oldVP);

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);
}

/** Draws a fullscreenquad, copying the given texture to the viewport */
void D3D11GraphicsEngine::DrawQuad(INT2 position, INT2 size)
{
	ID3D11ShaderResourceView* srv;
	Context->PSGetShaderResources(0, 1, &srv);

	ID3D11RenderTargetView* rtv;
	Context->OMGetRenderTargets(1, &rtv, NULL);

	if(srv)
	{
		if(rtv)
		{
			PfxRenderer->CopyTextureToRTV(srv, rtv, size, false, position);
			rtv->Release();
		}
		srv->Release();
	}
}


/** Sets the current rendering stage */
void D3D11GraphicsEngine::SetRenderingStage(D3D11ENGINE_RENDER_STAGE stage)
{
	RenderingStage = stage;
}

/** Returns the current rendering stage */
D3D11ENGINE_RENDER_STAGE D3D11GraphicsEngine::GetRenderingStage()
{
	return RenderingStage;
}

/** Draws a single VOB */
void D3D11GraphicsEngine::DrawVobSingle(VobInfo* vob)
{
	//vob->UpdateVobConstantBuffer();
	//vob->VobConstantBuffer->BindToVertexShader(1);

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	SetActivePixelShader("PS_Preview_Textured");
	SetActiveVertexShader("VS_Ex");

	SetDefaultStates();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable = false;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();
	//SetupVS_ExPerInstanceConstantBuffer();
	//vob->VobConstantBuffer->BindToVertexShader(1);

	//D3DXMATRIX id;
	//D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(vob->Vob->GetWorldMatrixPtr());
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = vob->VisualInfo->Meshes.begin(); itm != vob->VisualInfo->Meshes.end();itm++)
	{
		for(unsigned int i=0;i<(*itm).second.size();i++)
		{
			// Cache texture
			if((*itm).first)
			{
				if((*itm).first->GetTexture())
				{
					if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
					{
						(*itm).first->GetTexture()->Bind(0);

						MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom((*itm).first->GetTexture());
						if(!info->Constantbuffer)
						info->UpdateConstantbuffer();

						info->Constantbuffer->BindToPixelShader(2);
					}
					else
						continue;
				}
			}

			// Draw instances
			Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
		}
	}

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);
}

/** Message-Callback for the main window */
LRESULT D3D11GraphicsEngine::OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (UIView) {
		UIView->OnWindowMessage(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/** Draws the ocean */
XRESULT D3D11GraphicsEngine::DrawOcean(GOcean* ocean)
{
	SetDefaultStates();

	// Then draw the ocean
	SetActivePixelShader("PS_Ocean");
	SetActiveVertexShader("VS_ExDisplace");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();


	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	D3D11HDShader* hd = ShaderManager->GetHDShader("OceanTess");
	if(hd)hd->Apply();

	DefaultHullShaderConstantBuffer hscb;

	// convert to EdgesPerScreenHeight
	hscb.H_EdgesPerScreenHeight = GetResolution().y / Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	hscb.H_Proj11 = Engine::GAPI->GetRendererState()->TransformState.TransformProj._22;
	hscb.H_GlobalTessFactor = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	hscb.H_ScreenResolution = float2(GetResolution().x, GetResolution().y);
	hscb.H_FarPlane = Engine::GAPI->GetFarPlane();
	hd->GetConstantBuffer()[0]->UpdateBuffer(&hscb);
	hd->GetConstantBuffer()[0]->BindToHullShader(1);


	

	ID3D11ShaderResourceView* tex_displacement;
	ID3D11ShaderResourceView* tex_gradient;
	ID3D11ShaderResourceView* tex_fresnel;
	ID3D11ShaderResourceView* cube_reflect = ReflectionCube;
	OceanSettingsConstantBuffer ocb;
	ocean->GetFFTResources(&tex_displacement, &tex_gradient, &tex_fresnel, &ocb);
	ocb.OS_SunColor = Engine::GAPI->GetSky()->GetSunColor();


	if(tex_gradient)
		Context->PSSetShaderResources(0,1, &tex_gradient);

	if(tex_displacement)
	{
		Context->DSSetShaderResources(0,1, &tex_displacement);
	}

	Context->PSSetShaderResources(1,1, &tex_fresnel);
	Context->PSSetShaderResources(3,1, &cube_reflect);

	// Scene information is still bound from rendering water surfaces

	Context->PSSetSamplers(1, 1, &ClampSamplerState);
	Context->PSSetSamplers(2, 1, &CubeSamplerState);

	// Update constantbuffer
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&ocb);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(4);

	//DistortionTexture->BindToPixelShader(0);

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	ActivePS->GetConstantBuffer()[4]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[4]->BindToPixelShader(2);

	// Bind distortion texture
	DistortionTexture->BindToPixelShader(4);

	// Bind copied backbuffer
	Context->PSSetShaderResources(5, 1, PfxRenderer->GetTempBuffer()->GetShaderResViewPtr());

	// Unbind depth buffer
	//Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	// Bind depth to the shader
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	std::vector<D3DXVECTOR3> patches;
	ocean->GetPatchLocations(patches);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	D3DXMatrixTranspose(&view, &view);

	for(unsigned int i=0;i<patches.size();i++)
	{
		D3DXMATRIX scale, world;
		D3DXMatrixIdentity(&scale);
		D3DXMatrixScaling(&scale, (float)OCEAN_PATCH_SIZE, (float)OCEAN_PATCH_SIZE, (float)OCEAN_PATCH_SIZE);
		D3DXMatrixTranslation(&world, patches[i].x, patches[i].y, patches[i].z);
		world = scale * world;
		D3DXMatrixTranspose(&world, &world);
		ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&world);
		ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

		
		D3DXMATRIX invWorldView;
		D3DXVECTOR3 localEye = D3DXVECTOR3(0,0,0);
		D3DXMatrixTranspose(&world, &world);

		D3DXMatrixInverse(&invWorldView, NULL, &(world * view));
		D3DXVec3TransformCoord(&localEye, &localEye, &invWorldView);

		OceanPerPatchConstantBuffer opp;
		opp.OPP_LocalEye = localEye;
		opp.OPP_PatchPosition = patches[i];
		ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&opp);
		ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);

		
		ocean->GetPlaneMesh()->DrawMesh();
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Context->PSSetSamplers(2, 1, &ShadowmapSamplerState);

	SetActivePixelShader("PS_World");
	SetActiveVertexShader("VS_Ex");

	Context->HSSetShader(NULL, NULL, 0);
	Context->DSSetShader(NULL, NULL, 0);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return XR_SUCCESS;
}

/** Constructs the makro list for shader compilation */
void D3D11GraphicsEngine::ConstructShaderMakroList(std::vector<D3D10_SHADER_MACRO>& list)
{
	GothicRendererSettings& s = Engine::GAPI->GetRendererState()->RendererSettings;
	D3D10_SHADER_MACRO m;

	m.Name = "SHD_ENABLE";
	m.Definition = s.EnableShadows ? "1" : "0";
	list.push_back(m);

	m.Name = "SHD_FILTER_16TAP_PCF";
	m.Definition = s.EnableSoftShadows ? "1" : "0";
	list.push_back(m);

	m.Name = NULL;
	m.Definition = NULL;
	list.push_back(m);
}

/** Handles an UI-Event */
void D3D11GraphicsEngine::OnUIEvent(EUIEvent uiEvent)
{
	if(!UIView)
	{
		CreateMainUIView();
	}

	if(uiEvent == UI_OpenSettings)
	{	
		if(UIView)
		{
			// Show settings
			UIView->GetSettingsDialog()->SetHidden(!UIView->GetSettingsDialog()->IsHidden());

			// Free mouse
			Engine::GAPI->SetEnableGothicInput(UIView->GetSettingsDialog()->IsHidden());
		}
	}else if(uiEvent == UI_OpenEditor)
	{	
		if(UIView)
		{
			// Show settings
			Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel = true;

			// Free mouse
			Engine::GAPI->SetEnableGothicInput(UIView->GetSettingsDialog()->IsHidden());
		}
	}
}

/** Returns the data of the backbuffer */
void D3D11GraphicsEngine::GetBackbufferData(byte** data, int& pixelsize)
{
	byte* d = new byte[256 * 256* 4];

	// Copy HDR scene to backbuffer
	SetDefaultStates();

	SetActivePixelShader("PS_PFX_GammaCorrectInv");
	ActivePS->Apply();

	GammaCorrectConstantBuffer gcb;
	gcb.G_Gamma = Engine::GAPI->GetGammaValue();
	gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();
	gcb.G_TextureSize = GetResolution();
	gcb.G_SharpenStrength = Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor;

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	//PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), BackbufferRTV, INT2(0,0), true);
	
	HRESULT hr;

	// Buffer for scaling down the image
	RenderToTextureBuffer* rt = new RenderToTextureBuffer(Device, 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);

	// Downscale to 256x256
	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), rt->GetRenderTargetView(), INT2(256,256), true);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = 256; // Gothic transforms the backbufferdata for savegamethumbs to 256x256-pictures anyways
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D *texture;
	LE(Device->CreateTexture2D(&texDesc, 0, &texture));
	Context->CopyResource(texture, rt->GetTexture());

	// Get data
	D3D11_MAPPED_SUBRESOURCE res;
	if(SUCCEEDED(Context->Map(texture, 0, D3D11_MAP_READ, 0, &res)))
	{
		memcpy(d, res.pData, 256 * 256 * 4);
		Context->Unmap(texture, 0);
	}

	pixelsize = 4;
	*data = d;

	texture->Release();
	delete rt;
}

/** Binds the right shader for the given texture */
void D3D11GraphicsEngine::BindShaderForTexture(zCTexture* texture, bool forceAlphaTest, int zMatAlphaFunc)
{
	D3D11PShader* active = ActivePS;
	D3D11PShader* newShader = ActivePS;

	bool blendAdd = zMatAlphaFunc == zMAT_ALPHA_FUNC_ADD;
	bool blendBlend = zMatAlphaFunc == zMAT_ALPHA_FUNC_BLEND;
	bool linZ = (Engine::GAPI->GetRendererState()->GraphicsState.FF_GSwitches & GSWITCH_LINEAR_DEPTH) != 0;

	if(linZ)
	{
		newShader = PS_LinDepth;
	}else if(blendAdd || blendBlend)
	{
		newShader = PS_Simple;
	}else if(texture->HasAlphaChannel() || forceAlphaTest)
	{
		//if(texture->GetSurface()->GetNormalmap() 
		//	|| Engine::GAPI->GetSceneWetness()) // There is always a normalmap bound if the scene is wet, at least a basic one!
		//{
			if(texture->GetSurface()->GetFxMap())
			{
				newShader = PS_DiffuseNormalmappedAlphatestFxMap;
			}else
			{
				newShader = PS_DiffuseNormalmappedAlphatest;
			}
		//}else
		//{
		//	newShader = PS_DiffuseAlphatest;
		//}
	}else
	{
		//if(texture->GetSurface()->GetNormalmap()
		//	|| Engine::GAPI->GetSceneWetness()) // There is always a normalmap bound if the scene is wet, at least a basic one!)
		//{
			if(texture->GetSurface()->GetFxMap())
			{
				newShader = PS_DiffuseNormalmappedFxMap;
			}else
			{
				newShader = PS_DiffuseNormalmapped;
			}
		//}else
		//{
		//	newShader = PS_Diffuse;
		//}
	}

	// Bind, if changed
	if(active != newShader)
	{
		ActivePS = newShader;
		ActivePS->Apply();
	}
}

/** Draws the given list of decals */
void D3D11GraphicsEngine::DrawDecalList(const std::vector<zCVob *>& decals, bool lighting)
{
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXVECTOR3 camBack = -Engine::GAPI->GetCameraForward();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Set up alpha
	if(!lighting)
	{
		SetActivePixelShader("PS_Simple");
		Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	}else
	{
		SetActivePixelShader("PS_DiffuseAlphaTest");
	}
	
	SetActiveVertexShader("VS_Decal");
	
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	int lastAlphaFunc = -1;
	for(unsigned int i=0;i<decals.size();i++)
	{
		zCDecal* d = (zCDecal *)decals[i]->GetVisual();

		if(lighting && !d->GetAlphaTestEnabled())
			continue; // Only allow no alpha or alpha test

		if(!lighting)
		{
			switch(d->GetDecalSettings()->DecalMaterial->GetAlphaFunc())
			{
			case zMAT_ALPHA_FUNC_BLEND:
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;	
				break;

			case zMAT_ALPHA_FUNC_ADD:
				Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				break;

			/*case zRND_ALPHA_FUNC_MUL:
				Engine::GAPI->GetRendererState()->BlendState.SetModulateBlending();
				break;*/ // FIXME: Implement modulate

			default:
				continue; 
			}

			if(lastAlphaFunc != d->GetDecalSettings()->DecalMaterial->GetAlphaFunc())
			{
				Engine::GAPI->GetRendererState()->BlendState.SetDirty();
				UpdateRenderStates();
				lastAlphaFunc = d->GetDecalSettings()->DecalMaterial->GetAlphaFunc();
			}
		}

		
		int alignment = decals[i]->GetAlignment();

		D3DXMATRIX world;
		/*if(alignment == zVISUAL_CAM_ALIGN_YAW)
		{
			D3DXVECTOR3 up = D3DXVECTOR3(0,1,0);
			D3DXVECTOR3 side; D3DXVec3Cross(&side, &up, &camBack);
			D3DXVECTOR3 forward; D3DXVec3Cross(&forward, &side, &up);
 
			D3DXVECTOR4 m[] = {D3DXVECTOR4(side, 0), D3DXVECTOR4(up,0), D3DXVECTOR4(forward,0), D3DXVECTOR4(decals[i]->GetPositionWorld(), 1)};
			world = *(D3DXMATRIX *)m;
		}else*/
		{
			//D3DXMatrixIdentity(&world);
			decals[i]->GetWorldMatrix(&world);
			//D3DXVECTOR3 wp = decals[i]->GetPositionWorld();
			//D3DXMatrixTranslation(&world, wp.x, wp.y, wp.z);
		}

		D3DXMATRIX offset; D3DXMatrixTranslation(&offset, d->GetDecalSettings()->DecalOffset.x,
			-d->GetDecalSettings()->DecalOffset.y, 0);

		D3DXMATRIX scale; D3DXMatrixScaling(&scale, d->GetDecalSettings()->DecalSize.x * 2, -d->GetDecalSettings()->DecalSize.y * 2, 1);
		D3DXMatrixTranspose(&scale, &scale);
		

		//D3DXMatrixTranspose(&world, &world);
		D3DXMATRIX mat = view * world;

		if(alignment == zVISUAL_CAM_ALIGN_FULL || alignment == zVISUAL_CAM_ALIGN_YAW) 
		{
			for(int x=0;x<3;x++)
			{
				for(int y=0;y<3;y++) 
				{
					if ( x==y )
						mat(x,y) = 1.0;
					else
						mat(x,y) = 0.0;
				}
			}
		}

		mat = mat * offset * scale;

		//D3DXMatrixTranspose(&mat, &mat);

		//DrawVertexBufferIndexed(QuadVertexBuffer, QuadIndexBuffer, 6);

		ParticleInstanceInfo ii;
		ii.scale = D3DXVECTOR2(50,50);
		ii.color = 0xFFFFFFFF;

		Engine::GAPI->SetWorldTransform(mat);
		SetupVS_ExPerInstanceConstantBuffer();
		
		if(d->GetDecalSettings()->DecalMaterial)
		{
			if(d->GetDecalSettings()->DecalMaterial->GetTexture())
			{
				if(d->GetDecalSettings()->DecalMaterial->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
				{
					continue; // Don't render not cached surfaces
				}

				d->GetDecalSettings()->DecalMaterial->BindTexture(0);
			}
		}

		DrawVertexBufferIndexed(QuadVertexBuffer, QuadIndexBuffer, 6);
		//instances.push_back(ii);
	}

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;	
}


/** Draws quadmarks in a simple way */
void D3D11GraphicsEngine::DrawQuadMarks()
{
	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	const stdext::unordered_map<zCQuadMark*, QuadMarkInfo>& quadMarks = Engine::GAPI->GetQuadMarks();

	SetActiveVertexShader("VS_Ex");
	SetActivePixelShader("PS_World");

	SetDefaultStates();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	//Engine::GAPI->GetRendererState()->BlendState.SetModulateBlending();
	//Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	int alphaFunc = 0;
	for(stdext::unordered_map<zCQuadMark*, QuadMarkInfo>::const_iterator it = quadMarks.begin(); it != quadMarks.end(); it++)
	{
		if(!(*it).first->GetConnectedVob())
			continue;

		if(D3DXVec3Length(&(camPos - (*it).second.Position)) > Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius)
			continue;

		zCMaterial* mat = (*it).first->GetMaterial();
		if(mat)
			mat->BindTexture(0);

		if(alphaFunc != mat->GetAlphaFunc())
		{
			// Change alpha-func		
			switch(mat->GetAlphaFunc())
			{
			case zMAT_ALPHA_FUNC_ADD:
				Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				break;

			case zMAT_ALPHA_FUNC_BLEND:
				Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();
				break;

			case zMAT_ALPHA_FUNC_FUNC_NONE:
			case zMAT_ALPHA_FUNC_TEST:
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				break;

			/*case zRND_ALPHA_FUNC_MUL:
				Engine::GAPI->GetRendererState()->BlendState.SetModulateBlending();
				break;*/ // FIXME: Implement modulate

			default:
				continue;
			}

			alphaFunc = mat->GetAlphaFunc();

			Engine::GAPI->GetRendererState()->BlendState.SetDirty();
			UpdateRenderStates();
		}

		Engine::GAPI->SetWorldTransform(*(*it).first->GetConnectedVob()->GetWorldMatrixPtr());
		SetupVS_ExPerInstanceConstantBuffer();

		Engine::GraphicsEngine->DrawVertexBuffer((*it).second.Mesh, (*it).second.NumVertices);
	}
}

/** Copies the depth stencil buffer to DepthStencilBufferCopy */
void D3D11GraphicsEngine::CopyDepthStencil()
{
	Context->CopyResource(DepthStencilBufferCopy->GetTexture(), DepthStencilBuffer->GetTexture());
}

/** Draws underwater effects */
void D3D11GraphicsEngine::DrawUnderwaterEffects()
{
	SetDefaultStates();

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	// Set up water final copy
	SetActivePixelShader("PS_PFX_UnderwaterFinal");
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(3);

	DistortionTexture->BindToPixelShader(2);
	DepthStencilBufferCopy->BindToPixelShader(Context, 3);

	PfxRenderer->BlurTexture(HDRBackBuffer, false, 1.0f, UNDERWATER_COLOR_MOD, "PS_PFX_UnderwaterFinal");
}

/** Creates the main UI-View */
void D3D11GraphicsEngine::CreateMainUIView()
{
	if(!UIView)
	{
		UIView = new D2DView;

		ID3D11Texture2D* tex;
		BackbufferRTV->GetResource((ID3D11Resource **)&tex);
		if(XR_SUCCESS != UIView->Init(Resolution, tex))
		{
			delete UIView;
			UIView = NULL;

			if(tex)
				tex->Release();

			return;
		}

		if(tex)
			tex->Release();
	}
}

/** Sets up everything for a PNAEN-Mesh */
void D3D11GraphicsEngine::Setup_PNAEN(EPNAENRenderMode mode)
{
	D3D11HDShader* pnaen = ShaderManager->GetHDShader("PNAEN_Tesselation");

	if(mode == PNAEN_Instanced)
		SetActiveVertexShader("VS_PNAEN_Instanced");
	else if(mode == PNAEN_Default)
		SetActiveVertexShader("VS_PNAEN");
	else if(mode == PNAEN_Skeletal)
		SetActiveVertexShader("VS_PNAEN_Skeletal");

	ActiveVS->Apply();

	ActiveHDS = pnaen;
	pnaen->Apply();

	PNAENConstantBuffer cb;
	cb.f4Eye = Engine::GAPI->GetCameraPosition();
	cb.adaptive = INT4(1,0,0,0);
	cb.clipping = INT4(Engine::GAPI->GetRendererState()->RendererSettings.TesselationFrustumCulling ? 1 : 0,0,0,0);

	float f = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	cb.f4TessFactors = float4(f,f,Engine::GAPI->GetRendererState()->RendererSettings.TesselationRange, Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor);
	cb.f4ViewportScale.x = (float)GetResolution().x / 2;
	cb.f4ViewportScale.y = (float)GetResolution().y / 2;
	cb.f4x4Projection = Engine::GAPI->GetProjectionMatrix();

	pnaen->GetConstantBuffer()[0]->UpdateBuffer(&cb);

	pnaen->GetConstantBuffer()[0]->BindToDomainShader(0);
	pnaen->GetConstantBuffer()[0]->BindToHullShader(0);

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST);
}

/** Draws particle effects */
void D3D11GraphicsEngine::DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info)
{
	SetDefaultStates();



	// Clear GBuffer0 to hold the refraction vectors since it's not needed anymore
	Context->ClearRenderTargetView(GBuffer0_Diffuse->GetRenderTargetView(), (float *)&float4(0.0f,0.0f,0.0f,0.0f));
	Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&float4(0.0f,0.0f,0.0f,0.0f));


	// Bind it to the second slot
	//PfxRenderer->GetTempBuffer()->BindToPixelShader(Context, 2);

	D3D11PShader* distPS = ShaderManager->GetPShader("PS_ParticleDistortion");

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();
	ricb.RI_Far = Engine::GAPI->GetFarPlane();

	distPS->GetConstantBuffer()[0]->UpdateBuffer(&ricb);
	distPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GothicBlendStateInfo bsi = Engine::GAPI->GetRendererState()->BlendState;
	GothicRendererState& state = *Engine::GAPI->GetRendererState();

	state.BlendState.SetAdditiveBlending();
	state.BlendState.SetDirty();
	
	state.DepthState.DepthWriteEnabled = false;
	state.DepthState.SetDirty();
	state.DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::DEFAULT_DEPTH_COMP_STATE;
	
	state.RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	state.RasterizerState.SetDirty();

	std::vector<std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>> pvec;
	for(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>::iterator it = particles.begin(); it != particles.end();it++)
	{
		if((*it).second.empty())
			continue;
		
		pvec.push_back(std::make_tuple((*it).first, &info[(*it).first], &(*it).second));
	}

	struct cmp
	{
		static bool cmppt(const std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>& a,
						  const std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>& b)
		{
			// Sort additive before blend
			return std::get<1>(a)->BlendMode > std::get<1>(b)->BlendMode;
		}
	};

	// Sort additive before blend
	std::sort(pvec.begin(), pvec.end(), cmp::cmppt);

	SetActivePixelShader("PS_ParticleDistortion");
	ActivePS->Apply();

	ID3D11RenderTargetView* rtv[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	Context->OMSetRenderTargets(2, rtv, DepthStencilBuffer->GetDepthStencilView());

	int lastBlendMode = -1;

	// Bind view/proj
	SetupVS_ExConstantBuffer();

	// Setup GS
	GS_Billboard->Apply();

	ParticleGSInfoConstantBuffer gcb;
	gcb.CameraPosition = Engine::GAPI->GetCameraPosition();
	GS_Billboard->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	GS_Billboard->GetConstantBuffer()[0]->BindToGeometryShader(2);

	SetActiveVertexShader("VS_ParticlePoint");
	ActiveVS->Apply();

	// Rendering points only
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UpdateRenderStates();

	for(auto it = pvec.begin();it!=pvec.end();it++)
	{
		zCTexture* tx = std::get<0>((*it));
		ParticleRenderInfo& info = *std::get<1>((*it));
		std::vector<ParticleInstanceInfo>& instances = *std::get<2>((*it));

		if(instances.empty())
			continue;

		if(tx)
		{
			// Bind it
			if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
				tx->Bind(0);
			else
				continue;
		}

		GothicBlendStateInfo& blendState = info.BlendState;

		// This only happens once or twice, since the input list is sorted
		if(info.BlendMode != lastBlendMode)
		{
			// Setup blend state
			state.BlendState = blendState;
			state.BlendState.SetDirty();	

			lastBlendMode = info.BlendMode;

			if(info.BlendMode == zRND_ALPHA_FUNC_ADD)
			{
				// Set Distortion-Rendering for additive blending
				SetActivePixelShader("PS_ParticleDistortion");
				ActivePS->Apply();

				ID3D11RenderTargetView* rtv[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
				Context->OMSetRenderTargets(2, rtv, DepthStencilBuffer->GetDepthStencilView());
			}else
			{
				// Set usual rendering for everything else. Alphablending mostly.
				SetActivePixelShader("PS_Simple");
				PS_Simple->Apply();

				Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());
			}
		}

		// Push data for the particles to the GPU
		D3D11_BUFFER_DESC desc;
		((D3D11VertexBuffer *)TempVertexBuffer)->GetVertexBuffer()->GetDesc(&desc);

		if(desc.ByteWidth < sizeof(ParticleInstanceInfo) * instances.size())
		{
			LogInfo() << "(PARTICLE) TempVertexBuffer too small (" << desc.ByteWidth << "), need " << sizeof(ParticleInstanceInfo) * instances.size() << " bytes. Recreating buffer.";

			// Buffer too small, recreate it
			delete TempVertexBuffer;
			TempVertexBuffer = new D3D11VertexBuffer();

			TempVertexBuffer->Init(NULL, sizeof(ParticleInstanceInfo) * instances.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
		}

		TempVertexBuffer->UpdateBuffer(&instances[0], sizeof(ParticleInstanceInfo) * instances.size());
		
		DrawVertexBuffer(TempVertexBuffer, instances.size(), sizeof(ParticleInstanceInfo));
	}

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->GSSetShader(NULL, 0, 0);

	// Set usual rendertarget again
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	state.BlendState.SetDefault();
	state.BlendState.SetDirty();

	GBuffer0_Diffuse->BindToPixelShader(Context, 1);
	GBuffer1_Normals_SpecIntens_SpecPower->BindToPixelShader(Context, 2);

	// Copy scene behind the particle systems
	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), PfxRenderer->GetTempBuffer()->GetRenderTargetView());

	SetActivePixelShader("PS_PFX_ApplyParticleDistortion");
	ActivePS->Apply();

	// Copy it back, putting distortion behind it
	PfxRenderer->CopyTextureToRTV(PfxRenderer->GetTempBuffer()->GetShaderResView(), HDRBackBuffer->GetRenderTargetView(), INT2(0,0), true);


	SetDefaultStates();
}

/** Called when a vob was removed from the world */
XRESULT D3D11GraphicsEngine::OnVobRemovedFromWorld(zCVob* vob)
{
	if(UIView)
		UIView->GetEditorPanel()->OnVobRemovedFromWorld(vob);

	
	// Take out of shadowupdate queue
	for(auto it = FrameShadowUpdateLights.begin();it!=FrameShadowUpdateLights.end();it++)
	{
		if((*it)->Vob == vob)
		{
			FrameShadowUpdateLights.erase(it);
			break;
		}
	}

	DebugPointlight = NULL;

	return XR_SUCCESS;
}

/** Updates the occlusion for the bsp-tree */
void D3D11GraphicsEngine::UpdateOcclusion()
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.EnableOcclusionCulling)
		return;

	// Set up states
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = false; // Rasterization is faster without writes
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false; // Don't write the bsp-nodes to the depth buffer, also quicker
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	UpdateRenderStates();

	// Set up occlusion pass
	Occlusion->AdvanceFrameCounter();
	Occlusion->BeginOcclusionPass();

	// Do occlusiontests for the BSP-Tree
	Occlusion->DoOcclusionForBSP(Engine::GAPI->GetNewRootNode());

	Occlusion->EndOcclusionPass();

	SetDefaultStates();
}

/** Saves a screenshot */
void D3D11GraphicsEngine::SaveScreenshot()
{
	HRESULT hr;

	// Buffer for scaling down the image
	RenderToTextureBuffer* rt = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM);

	// Downscale to 256x256
	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), rt->GetRenderTargetView());

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = Resolution.x;  // must be same as backbuffer
	texDesc.Height = Resolution.y; // must be same as backbuffer
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D *texture;
	LE(Device->CreateTexture2D(&texDesc, 0, &texture));
	Context->CopyResource(texture, rt->GetTexture());

	char date[50];
	char time[50];

	// Format the filename
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL,"dd_MM_yyyy",date,50);
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, "hh_mm_ss", time, 50);

	// Create new folder if needed
	CreateDirectory("system\\Screenshots", NULL);

	std::string name = "system\\screenshots\\G2D3D11_" + std::string(date) + "__" + std::string(time) + ".jpg";

	LogInfo() << "Saving screenshot to: " << name;

	LE( D3DX11SaveTextureToFile(Context, texture, D3DX11_IFF_JPG, name.c_str()) );
	texture->Release();

	delete rt;

	// Inform the user that a screenshot has been taken
	Engine::GAPI->PrintMessageTimed(INT2(30,30), "Screenshot taken: " + name);
}