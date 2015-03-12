#pragma once
#include "pch.h"
#include "BasicTimer.h"

/** Struct handling all the graphical states set by the game. Can be used as Constantbuffer */
const int GSWITCH_FOG = 1;
const int GSWITCH_ALPHAREF = 2;
const int GSWITCH_LIGHING = 4;
const int GSWITCH_REFLECTIONS = 8;

/** A single fixed function stage */
struct FixedFunctionStage
{
	enum EColorOp
	{
		CO_DISABLE = 1,
		CO_SELECTARG1 = 2,
		CO_SELECTART2 = 3,

		CO_MODULATE = 4,
		CO_MODULATE2X = 5,
		CO_MODULATE4X = 6,

		CO_ADD = 7,
		CO_SUBTRACT = 10
	};

	enum ETextureArg
	{
		TA_DIFFUSE = 0,
		TA_CURRENT = 1,
		TA_TEXTURE = 2,
		TA_TFACTOR = 3
	};

	/** Sets the default values for this struct */
	void SetDefault()
	{
		
	}


	EColorOp ColorOp;
	ETextureArg ColorArg1;
	ETextureArg ColorArg2;

	EColorOp AlphaOp;
	ETextureArg AlphaArg1;
	ETextureArg AlphaArg2;

	int FFS_Pad1;
	int FFS_Pad2;
};



struct GothicGraphicsState
{
	/** Sets the default values for this struct */
	void SetDefault()
	{
		FF_FogWeight = 0.0f;
		FF_FogColor = float3(1.0f,1.0f,1.0f);
		FF_FogNear = 1.0f;
		FF_FogFar = 10000.0f;

		FF_AmbientLighting = float3(1.0f,1.0f,1.0f);
		FF_TextureFactor = float4(1.0f,1.0f,1.0f,1.0f);

		FF_AlphaRef = 0.5f;

		FF_GSwitches = 0;
	}

	/** Sets one of the GraphicsFlags */
	void SetGraphicsSwitch(int flag, bool enabled)
	{
		if(enabled)
			FF_GSwitches |= flag;
		else
			FF_GSwitches &= ~flag;
	}

	/** Fog section */
	float FF_FogWeight;
	float3 FF_FogColor;

	float FF_FogNear;
	float FF_FogFar;
	float2 ggs_Pad1;

	/** Lighting section */
	float3 FF_AmbientLighting;
	float FF_Time;

	/** Texture factor section */
	float4 FF_TextureFactor;

	/** Alpha ref section */
	float FF_AlphaRef;

	/** Graphical Switches (Takes GSWITCH_*) */
	unsigned int FF_GSwitches;
	float2 ggs_Pad3;

	FixedFunctionStage FF_Stages[2];
};



/** Depth buffer state information */
struct GothicDepthBufferStateInfo
{
	/** Layed out for D3D11 */
	enum ECompareFunc
	{
		CF_COMPARISON_NEVER	= 1,
		CF_COMPARISON_LESS	= 2,
		CF_COMPARISON_EQUAL	= 3,
		CF_COMPARISON_LESS_EQUAL	= 4,
		CF_COMPARISON_GREATER	= 5,
		CF_COMPARISON_NOT_EQUAL	= 6,
		CF_COMPARISON_GREATER_EQUAL	= 7,
		CF_COMPARISON_ALWAYS	= 8
	};

	/** Sets the default values for this struct */
	void SetDefault()
	{
		DepthBufferEnabled = true;
		DepthWriteEnabled = true;
		DepthBufferCompareFunc = CF_COMPARISON_LESS_EQUAL;
	}

	/** Depthbuffer settings */
	bool DepthBufferEnabled;
	bool DepthWriteEnabled;
	ECompareFunc DepthBufferCompareFunc;
};

/** Blend state information */
struct GothicBlendStateInfo
{
	/** Layed out for D3D11 */
	enum EBlendFunc
	{
		BF_ZERO	= 1,
        BF_ONE	= 2,
        BF_SRC_COLOR	= 3,
        BF_INV_SRC_COLOR	= 4,
        BF_SRC_ALPHA	= 5,
        BF_INV_SRC_ALPHA	= 6,
        BF_DEST_ALPHA	= 7,
        BF_INV_DEST_ALPHA	= 8,
        BF_DEST_COLOR	= 9,
        BF_INV_DEST_COLOR	= 10,
        BF_SRC_ALPHA_SAT	= 11,
        BF_BLEND_FACTOR	= 14,
        BF_INV_BLEND_FACTOR	= 15,
        BF_SRC1_COLOR	= 16,
        BF_INV_SRC1_COLOR	= 17,
        BF_SRC1_ALPHA	= 18,
        BF_INV_SRC1_ALPHA	= 19
	};

	/** Layed out for D3D11 */
	enum EBlendOp
	{
		BO_BLEND_OP_ADD	= 1,
		BO_BLEND_OP_SUBTRACT	= 2,
		BO_BLEND_OP_REV_SUBTRACT	= 3,
		BO_BLEND_OP_MIN	= 4,
		BO_BLEND_OP_MAX	= 5
	};

	/** Sets the default values for this struct */
	void SetDefault()
	{
		SrcBlend = BF_SRC_ALPHA;
		DestBlend = BF_INV_SRC_ALPHA;
		BlendOp = BO_BLEND_OP_ADD;
		SrcBlendAlpha = BF_ONE;
		DestBlendAlpha = BF_ZERO;
		BlendOpAlpha =	BO_BLEND_OP_ADD;
		BlendEnabled = false;
		AlphaToCoverage = false;
		ColorWritesEnabled = true;
	}

	/** Sets up alphablending */
	void SetAlphaBlending()
	{
		SrcBlend = BF_SRC_ALPHA;
		DestBlend = BF_INV_SRC_ALPHA;
		BlendOp = BO_BLEND_OP_ADD;
		SrcBlendAlpha = BF_ONE;
		DestBlendAlpha = BF_ZERO;
		BlendOpAlpha =	BO_BLEND_OP_ADD;
		BlendEnabled = true;
		AlphaToCoverage = false;
		ColorWritesEnabled = true;
	}

	/** Sets up additive blending */
	void SetAdditiveBlending()
	{
		SrcBlend = BF_SRC_ALPHA;
		DestBlend = BF_ONE;
		BlendOp = BO_BLEND_OP_ADD;
		SrcBlendAlpha = BF_ONE;
		DestBlendAlpha = BF_ZERO;
		BlendOpAlpha =	BO_BLEND_OP_ADD;
		BlendEnabled = true;
		AlphaToCoverage = false;
	}

	/** Sets up modualte blending */
	void SetModulateBlending()
	{
		SrcBlend = BF_SRC_COLOR;
		DestBlend = BF_DEST_COLOR;
		BlendOp = BO_BLEND_OP_ADD;
		SrcBlendAlpha = BF_ONE;
		DestBlendAlpha = BF_ZERO;
		BlendOpAlpha =	BO_BLEND_OP_ADD;
		BlendEnabled = true;
		AlphaToCoverage = false;
	}

	EBlendFunc SrcBlend;
	EBlendFunc DestBlend;
	EBlendOp BlendOp;
	EBlendFunc SrcBlendAlpha;
	EBlendFunc DestBlendAlpha;
	EBlendOp BlendOpAlpha;
	bool BlendEnabled;
	bool AlphaToCoverage;
	bool ColorWritesEnabled;
};

/** Blend state information */
struct GothicRasterizerStateInfo
{
	/** Layed out for D3D11 */
	enum ECullMode
	{
		CM_CULL_NONE	= 1,
		CM_CULL_FRONT	= 2,
		CM_CULL_BACK	= 3
	};



	/** Sets the default values for this struct */
	void SetDefault()
	{
		CullMode = CM_CULL_BACK;
		ZBias = 0;
		FrontCounterClockwise = false;
		Wireframe = false;
		
	}

	ECullMode CullMode;
	bool FrontCounterClockwise;
	int ZBias;
	bool Wireframe;
	
};

/** Sampler state information */
struct GothicSamplerStateInfo
{
	/** Layed out for D3D11 */
	enum ETextureAddress
	{
		TA_WRAP	= 1,
		TA_MIRROR = 2,
		TA_CLAMP = 3,
		TA_BORDER = 4,
		TA_MIRROR_ONCE = 5
	};

	/** Sets the default values for this struct */
	void SetDefault()
	{
		AddressU = TA_WRAP;
		AddressV = TA_WRAP;
	}

	ETextureAddress AddressU;
	ETextureAddress AddressV;
};

/** Transforms set by gothic. All of these must be transposed before sent to a shader! */
struct GothicTransformInfo
{
	/** Sets the default values for this struct */
	void SetDefault()
	{
		D3DXMatrixIdentity(&TransformWorld);
		D3DXMatrixIdentity(&TransformView);
		D3DXMatrixIdentity(&TransformProj);
	}

	/** This is actually world * view. Gothic never sets the view matrix */
	D3DXMATRIX TransformWorld; 

	/** Though never really set by Gothic, it's listed here for completeness sake */
	D3DXMATRIX TransformView;

	/** Projectionmatrix */
	D3DXMATRIX TransformProj;
};

struct HBAOSettings
{
	HBAOSettings()
	{
		MetersToViewSpaceUnits = 100.0f;
		Radius = 1.00f;
		Bias = 0.1f;
		PowerExponent = 3.0f;
		BlurSharpness = 4.0f;
		BlendMode = 1;
		Enabled = true;
	}

	float Bias;
	float PowerExponent;
	float BlurSharpness;
	float Radius;
	float MetersToViewSpaceUnits;
	int BlendMode;
	bool Enabled;
};

struct GothicRendererSettings
{
	/** Sets the default values for this struct */
	void SetDefault()
	{
		SectionDrawRadius = 3;

		DrawVOBs = true;
		DrawWorldMesh = true;
		DrawSkeletalMeshes = true;	
		DrawParticleEffects = true;
		DrawSky = true;
		DrawFog = true;
		EnableHDR = false;
		ReplaceSunDirection = false;
		AtmosphericScattering = true;
		EnableDynamicLighting = true;
		EnableAutoupdates = true;
		FastShadows = false;
		MaxNumFaces = 0;
		IndoorVobDrawRadius = 3000.0f;
		OutdoorVobDrawRadius = 25000.0f;
		SkeletalMeshDrawRadius = 6000.0f;
		VisualFXDrawRadius = 8000.0f;
		OutdoorSmallVobDrawRadius = 8000.0f;
		SmallVobSize = 1500.0f;

		

		FogGlobalDensity = 0.00014f;
		FogHeightFalloff = 0.0005f;

#ifdef BUILD_GOTHIC_1_08k
		FogGlobalDensity = 0.00051f;
		FogHeightFalloff = 0.00037f;
#endif

		FogHeight = 800;
		FogColorMod = float3::FromColor(180,180,255);
		SunLightColor = float3::FromColor(255,255,255);
		SunLightStrength = 1.5f;

		HDRLumWhite = 11.2f;
		HDRMiddleGray = 0.8f;
		BloomThreshold = 0.9f;

		WireframeVobs = false;
		WireframeWorld = false;
		DrawShadowGeometry = true;
		FixViewFrustum = false;
		DisableWatermark = false;
		DisableRendering = false;
		EnableEditorPanel = false;
		EnableSMAA = true;

		TesselationFactor = 20.0f;
		TesselationRange = 4000.0f;

		ShadowMapSize = 2048;
		WorldShadowRangeScale = 8.0f;

		BloomStrength = 1.0f;
		GlobalWindStrength = 1.0f;
		VegetationAlphaToCoverage = true;

		EnableSoftShadows = true;
		EnableShadows = true;
		EnableVSync = false;
		DoZPrepass = true;

		FOVHoriz = 90.0f;
		FOVVert = 90.0f;

		RECT desktopRect;
		GetClientRect(GetDesktopWindow(), &desktopRect);

		// Match the resolution with the current desktop resolution
		LoadedResolution = INT2(desktopRect.right, desktopRect.bottom);
		

		GothicUIScale = 1.0f;
		//DisableEverything();
	}

	void DisableEverything()
	{
		
	}

	/** Rendering options */
	bool DrawVOBs;
	bool DrawWorldMesh;
	bool DrawSkeletalMeshes;
	bool DrawParticleEffects;
	bool DrawSky;
	bool DrawFog;
	bool EnableHDR;
	bool EnableVSync;
	bool EnableSMAA;
	bool FastShadows;
	bool ReplaceSunDirection;
	bool AtmosphericScattering;
	bool EnableDynamicLighting;
	bool WireframeWorld;
	bool WireframeVobs;
	bool EnableSoftShadows;
	bool EnableShadows;
	bool DrawShadowGeometry;
	bool VegetationAlphaToCoverage;
	bool DisableWatermark;
	bool DisableRendering;
	bool EnableEditorPanel;
	bool DoZPrepass;
	bool EnableAutoupdates;

	int MaxNumFaces;

	int SectionDrawRadius;
	float IndoorVobDrawRadius;
	float OutdoorVobDrawRadius;
	float SkeletalMeshDrawRadius;
	float OutdoorSmallVobDrawRadius;
	float VisualFXDrawRadius;
	float SmallVobSize;
	float WorldShadowRangeScale;
	int ShadowMapSize;

	float GlobalWindStrength;
	float FogGlobalDensity;
	float FogHeightFalloff;
	float FogHeight;
	float3 FogColorMod;
	float3 SunLightColor;
	float SunLightStrength;
	INT2 LoadedResolution;

	float TesselationFactor;
	float TesselationRange;
	float HDRLumWhite;
	float HDRMiddleGray;
	float BloomThreshold;
	float BloomStrength;
	float GothicUIScale;
	float FOVHoriz;
	float FOVVert;

	HBAOSettings HbaoSettings;

	bool FixViewFrustum;
};

struct GothicRendererTiming
{
	enum TIME_TYPE
	{
		TT_WorldMesh,
		TT_Vobs,
		TT_Lighting,
		TT_SkeletalMeshes
	};

	void Start()
	{
		_timer.Update();
	}

	void Stop(TIME_TYPE tt)
	{
		_timer.Update();

		switch(tt)
		{
		case TT_WorldMesh:
			WorldMeshMS = _timer.GetDelta() * 1000.0f;
			break;

		case TT_Vobs:
			VobsMS = _timer.GetDelta() * 1000.0f;
			break;

		case TT_Lighting:
			LightingMS = _timer.GetDelta() * 1000.0f;
			break;

		case TT_SkeletalMeshes:
			SkeletalMeshesMS = _timer.GetDelta()*  1000.0f;
			break;
		}
	}

	void StartTotal()
	{
		_totalTimer.Update();
	}

	void StopTotal()
	{
		_totalTimer.Update();
		TotalMS = _totalTimer.GetDelta() * 1000.0f;
	}

	float WorldMeshMS;
	float VobsMS;
	float LightingMS;
	float SkeletalMeshesMS;
	float TotalMS;

private:
	BasicTimer _timer;
	BasicTimer _totalTimer;
};

struct GothicRendererInfo
{
	GothicRendererInfo()
	{
		VOBVerticesDataSize = 0;
		SkeletalVerticesDataSize = 0;
	}

	void Reset()
	{
		FrameDrawnTriangles = 0;
		FrameDrawnVobs = 0;
		FPS = 0;
		FrameVobUpdates = 0;
		FrameNumSectionsDrawn = 0;

		FarPlane = 0;
		NearPlane = 0;	
		FrameDrawnLights = 0;
		WorldMeshDrawCalls = 0;
	}

	int FrameDrawnTriangles;
	int FrameDrawnVobs;
	int FrameVobUpdates;
	int FrameNumSectionsDrawn;
	int FPS;
	float FarPlane;
	float NearPlane;
	int FrameDrawnLights;
	int WorldMeshDrawCalls;

	GothicRendererTiming Timing;

	unsigned int VOBVerticesDataSize;
	unsigned int SkeletalVerticesDataSize;
};

/** This handles more device specific settings */
struct GothicRendererState
{
	GothicRendererState()
	{
		DepthState.SetDefault();
		BlendState.SetDefault();
		RasterizerState.SetDefault();
		GraphicsState.SetDefault();
		SamplerState.SetDefault();
		TransformState.SetDefault();
		RendererSettings.SetDefault();

		DepthStateDirty = false;
		BlendStateDirty = false;
		RasterizerStateDirty = false;
		SamplerStateDirty = false;


	}

	GothicDepthBufferStateInfo DepthState;
	bool DepthStateDirty;

	GothicBlendStateInfo BlendState;
	bool BlendStateDirty;

	GothicRasterizerStateInfo RasterizerState;
	bool RasterizerStateDirty;

	GothicSamplerStateInfo SamplerState;
	bool SamplerStateDirty;

	GothicGraphicsState GraphicsState;
	GothicTransformInfo TransformState;
	GothicRendererSettings RendererSettings;
	GothicRendererInfo RendererInfo;
};