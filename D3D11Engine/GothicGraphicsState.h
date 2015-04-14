#pragma once
#include "pch.h"
#include "BasicTimer.h"
#include "BasePipelineStates.h"

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

__declspec(align(4)) struct GothicPipelineState
{
	/** Sets this state dirty, which means that it will be updated before next rendering */
	void SetDirty()
	{
		StateDirty = true;
		HashThis((char *)this, StructSize); 
	}

	/** Hashes the whole struct */
	void HashThis(char* data, int size)
	{
		Hash = 0;

		// Start hashing at the data of the other structs, skip the data of this one
		for(int i=sizeof(GothicPipelineState);i<size;i+=4)
		{
			DWORD d;
			((char *)&d)[0] = data[i];
			((char *)&d)[1] = data[i+1];
			((char *)&d)[2] = data[i+2];
			((char *)&d)[3] = data[i+3];

			Toolbox::hash_combine(Hash, d);
		}
	}

	bool operator == (const GothicPipelineState& o) const
	{
		return Hash == o.Hash;
	}

	bool StateDirty;
	size_t Hash;
	int StructSize;
};

struct GothicPipelineKeyHasher
{
	static const size_t bucket_size = 10; // mean bucket size that the container should try not to exceed
	static const size_t min_buckets = (1 << 10); // minimum number of buckets, power of 2, >0

	static std::size_t hash_value(float value)
	{
		stdext::hash<float> hasher;
		return hasher(value);
	}

	static void hash_combine(std::size_t& seed, float value)
	{	
		seed ^= hash_value(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	std::size_t operator()(const GothicPipelineState& k) const
	{
		return k.Hash;
	}

};

namespace GothicStateCache
{
	/** Hashmap for caching the state-objects */
	__declspec(selectany) std::unordered_map<GothicDepthBufferStateInfo, BaseDepthBufferState*, GothicPipelineKeyHasher> s_DepthBufferMap;
	__declspec(selectany) std::unordered_map<GothicBlendStateInfo, BaseBlendStateInfo*, GothicPipelineKeyHasher> s_BlendStateMap;
	__declspec(selectany) std::unordered_map<GothicRasterizerStateInfo, BaseRasterizerStateInfo*, GothicPipelineKeyHasher> s_RasterizerStateMap;
};

/** Depth buffer state information */
class BaseDepthBufferState;
struct GothicDepthBufferStateInfo : public GothicPipelineState
{
	GothicDepthBufferStateInfo()
	{
		StructSize = sizeof(GothicDepthBufferStateInfo);
	}

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

	/** Deletes all cached states */
	static void DeleteCachedObjects()
	{
		for(auto it = GothicStateCache::s_DepthBufferMap.begin();it != GothicStateCache::s_DepthBufferMap.end();it++)
		{
			delete (*it).second;
		}

		GothicStateCache::s_DepthBufferMap.clear();
	}

};

/** Blend state information */
class BaseBlendStateInfo;
struct GothicBlendStateInfo : public GothicPipelineState
{
	GothicBlendStateInfo()
	{
		StructSize = sizeof(GothicBlendStateInfo);
	}

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
		SrcBlend = BF_DEST_COLOR;
		DestBlend = BF_ZERO;
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


	/** Deletes all cached states */
	static void DeleteCachedObjects()
	{
		for(auto it = GothicStateCache::s_BlendStateMap.begin();it != GothicStateCache::s_BlendStateMap.end();it++)
		{
			delete (*it).second;
		}

		GothicStateCache::s_BlendStateMap.clear();
	}

};

/** Blend state information */
class BaseRasterizerStateInfo;
struct GothicRasterizerStateInfo : public GothicPipelineState
{
	GothicRasterizerStateInfo()
	{
		StructSize = sizeof(GothicRasterizerStateInfo);
	}

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
	
	/** Deletes all cached states */
	static void DeleteCachedObjects()
	{
		for(auto it = GothicStateCache::s_RasterizerStateMap.begin();it != GothicStateCache::s_RasterizerStateMap.end();it++)
		{
			delete (*it).second;
		}

		GothicStateCache::s_RasterizerStateMap.clear();
	}
};

/** Sampler state information */
struct GothicSamplerStateInfo : public GothicPipelineState
{
	GothicSamplerStateInfo()
	{
		StructSize = sizeof(GothicSamplerStateInfo);
	}

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
		SectionDrawRadius = 4;

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
		IndoorVobDrawRadius = 5000.0f;
		OutdoorVobDrawRadius = 30000.0f;
		SkeletalMeshDrawRadius = 6000.0f;
		VisualFXDrawRadius = 8000.0f;
		OutdoorSmallVobDrawRadius = 10000.0f;
		SmallVobSize = 1500.0f;

		

		

#ifdef BUILD_GOTHIC_1_08k
		FogGlobalDensity = 0.00002f;
		FogHeightFalloff = 0.00018f;

		FogColorMod = float3::FromColor(189,146,107);
		FogHeight = 4000;
#else
		FogGlobalDensity = 0.00004f;
		FogHeightFalloff = 0.0005f;
		FogColorMod = float3::FromColor(180,180,255);
		FogHeight = 800;
#endif

		

		
		SunLightColor = float3::FromColor(255,255,255);
		SunLightStrength = 1.5f;

		HDRLumWhite = 11.2f;
		HDRMiddleGray = 0.8f;
		BloomThreshold = 0.9f;

		WireframeVobs = false;
		WireframeWorld = false;
		DrawShadowGeometry = true;
		FixViewFrustum = false;
		DisableWatermark = true;
		DisableRendering = false;
		EnableEditorPanel = false;
		EnableSMAA = true;

		TesselationFactor = 20.0f;
		TesselationRange = 4000.0f;

		ShadowMapSize = 2048;
		WorldShadowRangeScale = 8.0f;

		ShadowStrength = 0.40f;
		ShadowAOStrength = 0.50f;
		WorldAOStrength = 0.50f;

		BloomStrength = 1.0f;
		GlobalWindStrength = 1.0f;
		VegetationAlphaToCoverage = true;

		BrightnessValue = 1.0f;
		GammaValue = 1.0f;

		EnableSoftShadows = true;
		EnableShadows = true;
		EnableVSync = false;
		DoZPrepass = true;

		FOVHoriz = 90.0f;
		FOVVert = 90.0f;

		GodRayDecay = 0.97f;
		GodRayWeight = 0.85f;
		GodRayDensity = 0.70f;
		GodRayColorMod = float3(1.0f, 0.7f, 0.8f);

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
	float GammaValue;
	float BrightnessValue;
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

	float ShadowStrength;
	float ShadowAOStrength;
	float WorldAOStrength;

	float GodRayDecay;
	float GodRayWeight;
	float GodRayDensity;
	float3 GodRayColorMod;

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
		PlayingMovieResolution = INT2(0,0);
		Reset();
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

	/** Resolution of the currently playing video, only valid when a movie plays! */
	INT2 PlayingMovieResolution;
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

		DepthState.SetDirty();
		BlendState.SetDirty();
		RasterizerState.SetDirty(); 
		SamplerState.SetDirty();


	}

	GothicDepthBufferStateInfo DepthState;

	GothicBlendStateInfo BlendState;

	GothicRasterizerStateInfo RasterizerState;

	GothicSamplerStateInfo SamplerState;

	GothicGraphicsState GraphicsState;
	GothicTransformInfo TransformState;
	GothicRendererSettings RendererSettings;
	GothicRendererInfo RendererInfo;
};