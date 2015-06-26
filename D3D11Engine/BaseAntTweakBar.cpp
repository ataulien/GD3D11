#include "pch.h"
#include "BaseAntTweakBar.h"
#include "AntTweakBar.h"
#include "GSky.h"
#include "GothicAPI.h"
#include "Engine.h"
#include "zCMaterial.h"
#include "BaseGraphicsEngine.h"
#include <algorithm>

#pragma comment(lib, "AntTweakBar.lib")

BaseAntTweakBar::BaseAntTweakBar(void)
{
	IsActive = false;
	Bar_Sky = NULL;
	TS_Active = false;
	TS_TextureName[0] = 0;
	ActiveMaterialInfo = NULL;
	LastFrameActiveMaterialInfo = NULL;
}


BaseAntTweakBar::~BaseAntTweakBar(void)
{
	//TwTerminate();
}

/** Creates the resources */
XRESULT BaseAntTweakBar::Init()
{
	// Sky
	Bar_Sky = TwNewBar("Sky");
	TwDefine(" Sky position='400 0'");

	TwAddVarRW(Bar_Sky, "GodRays", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableGodRays, NULL);

	TwAddVarRW(Bar_Sky, "GodRayDecay", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.GodRayDecay, NULL);
	TwDefine(" Sky/GodRayDecay  step=0.01 ");
	
	TwAddVarRW(Bar_Sky, "GodRayWeight", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.GodRayWeight, NULL);
	TwDefine(" Sky/GodRayWeight  step=0.01 ");
	
	TwAddVarRW(Bar_Sky, "GodRayColorMod", TW_TYPE_COLOR3F, &Engine::GAPI->GetRendererState()->RendererSettings.GodRayColorMod, NULL);

	TwAddVarRW(Bar_Sky, "GodRayDensity", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.GodRayDensity, NULL);
	TwDefine(" Sky/GodRayWeight  step=0.01 ");

	TwAddSeparator(Bar_Sky, "SkySettings", NULL);

	TwAddVarRW(Bar_Sky, "G", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().G, NULL);
	TwDefine(" Sky/G  step=0.01 ");
	TwDefine(" Sky/G  help='Size of the Sun' ");

	TwAddVarRW(Bar_Sky, "RayleightScaleDepth", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().RayleightScaleDepth, NULL);
	TwDefine(" Sky/RayleightScaleDepth  step=0.01 ");
	TwDefine(" Sky/RayleightScaleDepth  help='Controls scattering' ");

	TwAddVarRW(Bar_Sky, "ESun", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().ESun, NULL);
	TwDefine(" Sky/ESun  step=0.1 ");
	TwDefine(" Sky/ESun  help='Brightness of the sun' ");

	TwAddVarRW(Bar_Sky, "InnerRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().InnerRadius, NULL);
	TwAddVarRW(Bar_Sky, "OuterRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().OuterRadius, NULL);
	TwDefine(" Sky/InnerRadius  help='Inner Radius of the fake-planet. This must be greater than SphereOffset.y.' ");
	TwDefine(" Sky/OuterRadius  help='Outer Radius of the fake-planet' ");

	TwAddVarRW(Bar_Sky, "Km", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().Km, NULL);
	TwDefine(" Sky/Km  step=0.0001 ");

	TwAddVarRW(Bar_Sky, "Kr", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().Kr, NULL);
	TwDefine(" Sky/Kr  step=0.0001 ");

	TwAddVarRW(Bar_Sky, "Samples", TW_TYPE_INT32, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().Samples, NULL);
	TwAddVarRW(Bar_Sky, "WaveLengths", TW_TYPE_DIR3F, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().WaveLengths, NULL);
	TwAddVarRW(Bar_Sky, "SphereOffset.y", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().SphereOffsetY, NULL);

	TwAddVarRW(Bar_Sky, "ReplaceSunDirection", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.ReplaceSunDirection, NULL);
	TwDefine(" Sky/ReplaceSunDirection  help='Activates the LightDirection-Vector so you can modify the Direction of the sun' ");

	TwAddVarRW(Bar_Sky, "LightDirection", TW_TYPE_DIR3F, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection, NULL);
	TwDefine(	" Sky/ReplaceSunDirection  help='The direction the sun should come from. Only active when ReplaceSunDirection is active.\n"
				"Also useful to fix the sun in one position'");

	TwAddVarRW(Bar_Sky, "SunLightColor", TW_TYPE_COLOR3F, &Engine::GAPI->GetRendererState()->RendererSettings.SunLightColor, NULL);
	TwDefine(" Sky/SunLightColor  help='Color of the sunlight' ");

	TwAddVarRW(Bar_Sky, "SunLightStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.SunLightStrength, NULL);

	TwAddVarRW(Bar_Sky, "SkyTimeScale", TW_TYPE_FLOAT, &Engine::GAPI->GetSky()->GetAtmoshpereSettings().SkyTimeScale, NULL);
	TwDefine(" Sky/SkyTimeScale  help='This makes the skys time pass slower or faster' ");
	
	Bar_General = TwNewBar("General");
	TwDefine(" General position='600 0'");

	
	TwAddButton(Bar_General, "Save ZEN-Resources", (TwButtonCallback)SaveZENResourcesCallback, this, NULL); 
	TwAddButton(Bar_General, "Load ZEN-Resources", (TwButtonCallback)LoadZENResourcesCallback, this, NULL); 
	TwAddButton(Bar_General, "Open Settings Dialog", (TwButtonCallback)OpenSettingsCallback, this, NULL); 

	TwAddVarRW(Bar_General, "DisableRendering", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering, NULL);
	TwAddVarRW(Bar_General, "Draw VOBs", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs, NULL);

	TwAddVarRW(Bar_General, "Draw WorldMesh", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh, NULL);
	TwAddVarRW(Bar_General, "Draw Skeletal Meshes", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes, NULL);
	TwAddVarRW(Bar_General, "Draw Mobs", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawMobs, NULL);
	TwAddVarRW(Bar_General, "Draw ParticleEffects", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawParticleEffects, NULL);
	//TwAddVarRW(Bar_General, "Draw Sky", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawSky, NULL);
	TwAddVarRW(Bar_General, "Draw Fog", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawFog, NULL);	
	
#ifndef PUBLIC_RELEASE
	TwAddVarRW(Bar_General, "HDR (Broken!)", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableHDR, NULL);	
#endif

	TwAddVarRW(Bar_General, "SMAA", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA, NULL);
	TwAddVarRW(Bar_General, "Sharpen", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor, NULL);
	TwDefine(" General/Sharpen  step=0.01 min=0");

	TwAddVarRW(Bar_General, "DynamicLighting", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableDynamicLighting, NULL);	

	TwType epls = TwDefineEnumFromString("PointlightShadowsEnum", "0 {Disabled}, 1 {Static}, 2 {Update Dynamic}, 3 {Full}");
	TwAddVarRW(Bar_General, "PointlightShadows", epls, &Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows, NULL);

	//TwAddVarRW(Bar_General, "FastShadows", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.FastShadows, NULL);	
	TwAddVarRW(Bar_General, "DrawShadowGeometry", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DrawShadowGeometry, NULL);
	TwAddVarRW(Bar_General, "DoZPrepass", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DoZPrepass, NULL);

	TwAddVarRW(Bar_General, "Tesselation", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation, NULL);	

	TwAddVarRW(Bar_General, "VSync", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync, NULL);
	
	TwAddVarRW(Bar_General, "OcclusionCulling", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableOcclusionCulling, NULL);
	TwAddVarRW(Bar_General, "Sort RenderQueue", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.SortRenderQueue, NULL);
	
	TwAddVarRW(Bar_General, "AllowWorldMeshTesselation", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.AllowWorldMeshTesselation, NULL);
	TwAddVarRW(Bar_General, "TesselationFrustumCulling", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.TesselationFrustumCulling, NULL);

	

	TwType t = TwDefineEnumFromString("ShadowmapSizeEnum", "512, 1024, 2048, 4096");
	TwAddVarRW(Bar_General, "ShadowmapSize", t, &Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize, NULL);
	TwAddVarRW(Bar_General, "WorldShadowRangeScale", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, NULL);
	TwDefine(" General/WorldShadowRangeScale  step=0.01 min=0");

	TwAddVarRW(Bar_General, "ShadowStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.ShadowStrength, NULL);
	TwDefine(" General/ShadowStrength  step=0.01 min=0");

	TwAddVarRW(Bar_General, "ShadowAOStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.ShadowAOStrength, NULL);
	TwDefine(" General/ShadowAOStrength  step=0.01 min=0");

	TwAddVarRW(Bar_General, "WorldAOStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.WorldAOStrength, NULL);
	TwDefine(" General/WorldAOStrength  step=0.01 min=0");

	TwAddVarRW(Bar_General, "ShadowStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.ShadowStrength, NULL);
	TwDefine(" General/ShadowStrength  step=0.01 min=0");

		TwAddVarRW(Bar_General, "WireframeWorld", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld, NULL);	
	TwAddVarRW(Bar_General, "WireframeVobs", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs, NULL);	
	

	TwAddVarRW(Bar_General, "OldTesselationFactor", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor, NULL);
	TwDefine(" General/TesselationFactor  step=0.1 min=1");

	TwAddVarRW(Bar_General, "TesselationEdgeLength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.TesselationRange, NULL);
	TwDefine(" General/TesselationRange  step=0.1 min=1");

	

	//TwAddVarRW(Bar_General, "Grass AlphaToCoverage", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.VegetationAlphaToCoverage, NULL);	
	

	TwAddVarRW(Bar_General, "SectionDrawRadius", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius, NULL);
	TwDefine(" General/SectionDrawRadius  help='Draw distance for the sections' ");

	TwAddVarRW(Bar_General, "OutdoorVobDrawRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius, NULL);
	TwDefine(" General/OutdoorVobDrawRadius  help='Draw distance for the static outdoor vobs' ");

	TwAddVarRW(Bar_General, "IndoorVobDrawRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius, NULL);
	TwDefine(" General/OutdoorVobDrawRadius  help='Draw distance for the static indoor vobs' ");

	TwAddVarRW(Bar_General, "OutdoorSmallVobRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius, NULL);

	TwAddVarRW(Bar_General, "VisualFXDrawRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius, NULL);

	TwAddVarRW(Bar_General, "RainRadius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.RainRadiusRange, NULL);
	TwAddVarRW(Bar_General, "RainHeight", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.RainHeightRange, NULL);
	TwAddVarRW(Bar_General, "NumRainParticles", TW_TYPE_UINT32, &Engine::GAPI->GetRendererState()->RendererSettings.RainNumParticles, NULL);
	TwDefine(" General/NumRainParticles  step=1000");

	TwAddVarRW(Bar_General, "RainMoveParticles", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.RainMoveParticles, NULL);
	TwAddVarRW(Bar_General, "RainUseInitialSet", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.RainUseInitialSet, NULL);	
	
	TwAddVarRW(Bar_General, "RainGlobalVelocity", TW_TYPE_DIR3F, &Engine::GAPI->GetRendererState()->RendererSettings.RainGlobalVelocity, NULL);
	TwAddVarRW(Bar_General, "RainSceneWettness", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.RainSceneWettness, NULL);
	TwDefine(" General/RainSceneWettness  step=0.01 min=0 max=1");	

	TwAddVarRW(Bar_General, "RainSunLightStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.RainSunLightStrength, NULL);
	TwDefine(" General/RainSunLightStrength  step=0.01 min=0");

	TwAddVarRW(Bar_General, "RainFogDensity", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.RainFogDensity, NULL);
	TwDefine(" General/RainFogDensity  step=0.00001 min=0");

	TwAddVarRW(Bar_General, "RainFogColor", TW_TYPE_COLOR3F, &Engine::GAPI->GetRendererState()->RendererSettings.RainFogColor, NULL);

	//TwAddVarRW(Bar_General, "SmallVobSize", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize, NULL);
	//TwAddVarRW(Bar_General, "AtmosphericScattering", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.AtmosphericScattering, NULL);

	TwAddVarRW(Bar_General, "FogGlobalDensity", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.FogGlobalDensity, NULL);
	TwDefine(" General/FogGlobalDensity  step=0.00001 min=0");
	TwDefine(" General/FogGlobalDensity  help='Controls thickness of the fog' ");

	TwAddVarRW(Bar_General, "FogHeightFalloff", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.FogHeightFalloff, NULL);
	TwDefine(" General/FogHeightFalloff  step=0.00001 min=0");
	TwDefine(" General/FogHeightFalloff  help='Controls how sharp the falloff of the fog should be' ");

	TwAddVarRW(Bar_General, "FogHeight", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.FogHeight, NULL);
	TwDefine(" General/FogHeight  help='Height at where the Fog should start' ");

	TwAddVarRW(Bar_General, "FogColorMod", TW_TYPE_COLOR3F, &Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod, NULL);
	TwDefine(" General/FogColorMod  help='Color of the fog' ");

	TwAddVarRW(Bar_General, "HDRLumWhite", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HDRLumWhite, NULL);
	TwDefine(" General/HDRLumWhite  step=0.01");
	TwDefine(" General/HDRLumWhite");

	TwAddVarRW(Bar_General, "HDRMiddleGray", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HDRMiddleGray, NULL);
	TwDefine(" General/HDRMiddleGray  step=0.01");
	TwDefine(" General/HDRMiddleGray");

	TwAddVarRW(Bar_General, "BloomThreshold", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.BloomThreshold, NULL);
	TwDefine(" General/BloomThreshold  step=0.01");
	TwDefine(" General/BloomThreshold");

	TwAddVarRW(Bar_General, "BloomStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.BloomStrength, NULL);
	TwDefine(" General/BloomStrength  step=0.01");
	TwDefine(" General/BloomStrength");
	
	TwAddVarRW(Bar_General, "WindStrength", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.GlobalWindStrength, NULL);
	TwDefine(" General/WindStrength  step=0.01");

	TwAddVarRW(Bar_General, "LockViewFrustum", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum, NULL);
	TwAddVarRW(Bar_General, "DisableWatermark", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.DisableWatermark, NULL);
	
	TwAddVarRW(Bar_General, "GothicUIScale", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale, NULL);
	TwAddVarRW(Bar_General, "FOVHoriz", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.FOVHoriz, NULL);
	TwAddVarRW(Bar_General, "FOVVert", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.FOVVert, NULL);
	//TwAddVarRW(Bar_General, "Max Num Indices", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererSettings.MaxNumFaces, NULL);



	Bar_Info = TwNewBar("FrameStats");
	TwDefine(" FrameStats refresh=0.3");
	TwDefine(" FrameStats position='800 0'");
	//TwAddVarRO(Bar_Info, "Version", TW_TYPE_CDSTRING, VERSION_NUMBER_STR, NULL);
	TwAddVarRO(Bar_Info, "FPS", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FPS, NULL);
	TwAddVarRO(Bar_Info, "StateChanges", TW_TYPE_UINT32, &Engine::GAPI->GetRendererState()->RendererInfo.StateChanges, NULL);
	TwAddVarRO(Bar_Info, "DrawnVobs", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs, NULL);
	TwAddVarRO(Bar_Info, "DrawnTriangles", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles, NULL);
	TwAddVarRO(Bar_Info, "VobUpdates", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FrameVobUpdates, NULL);
	TwAddVarRO(Bar_Info, "DrawnLights", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnLights, NULL);
	TwAddVarRO(Bar_Info, "SectionsDrawn", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.FrameNumSectionsDrawn, NULL);
	TwAddVarRO(Bar_Info, "WorldMeshDrawCalls", TW_TYPE_INT32, &Engine::GAPI->GetRendererState()->RendererInfo.WorldMeshDrawCalls, NULL);
	
	TwAddVarRO(Bar_Info, "FarPlane", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.FarPlane, NULL);
	TwAddVarRO(Bar_Info, "NearPlane", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.NearPlane, NULL);
	//TwAddVarRO(Bar_Info, "VOBVerticesDataSize", TW_TYPE_UINT32, &Engine::GAPI->GetRendererState()->RendererInfo.VOBVerticesDataSize, NULL);
	//TwAddVarRO(Bar_Info, "SkeletalVerticesDataSize", TW_TYPE_UINT32, &Engine::GAPI->GetRendererState()->RendererInfo.SkeletalVerticesDataSize, NULL);

	TwAddVarRO(Bar_Info, "WorldMeshMS", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.Timing.WorldMeshMS, NULL);
	TwAddVarRO(Bar_Info, "VobsMS", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.Timing.VobsMS, NULL);
	TwAddVarRO(Bar_Info, "SkeletalMeshesMS", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.Timing.SkeletalMeshesMS, NULL);
	TwAddVarRO(Bar_Info, "LightingMS", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.Timing.LightingMS, NULL);
	TwAddVarRO(Bar_Info, "TotalMS", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererInfo.Timing.TotalMS, NULL);

	TwAddVarRO(Bar_Info, "SC_Textures,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_TX], NULL);
	TwAddVarRO(Bar_Info, "SC_GeometryShader,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_GS], NULL);
	TwAddVarRO(Bar_Info, "SC_RTVDSV,", TW_TYPE_UINT32,	&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_RTVDSV], NULL);
	TwAddVarRO(Bar_Info, "SC_DomainShader,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_DS], NULL);
	TwAddVarRO(Bar_Info, "SC_HullShader,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_HS], NULL);
	TwAddVarRO(Bar_Info, "SC_PixelShader,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_PS], NULL);
	TwAddVarRO(Bar_Info, "SC_InputLayout,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_IL], NULL);
	TwAddVarRO(Bar_Info, "SC_VertexShader,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_VS], NULL);
	TwAddVarRO(Bar_Info, "SC_IndexBuffer,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_IB], NULL);
	TwAddVarRO(Bar_Info, "SC_VertexBuffer,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_VB], NULL);
	TwAddVarRO(Bar_Info, "SC_RasterizerState,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_RS], NULL);
	TwAddVarRO(Bar_Info, "SC_DepthStencilState,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_DSS], NULL);
	TwAddVarRO(Bar_Info, "SC_SamplerState,", TW_TYPE_UINT32,	&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_SMPL], NULL);
	TwAddVarRO(Bar_Info, "SC_BlendState,", TW_TYPE_UINT32,		&Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[GothicRendererInfo::SC_BS], NULL);
					

	Bar_HBAO = TwNewBar("HBAO+");
	TwDefine(" HBAO+ position='1000 0'");
	
	/** --- hbao  --- */	
	TwAddVarRW(Bar_HBAO, "Enable HBAO+", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Enabled, NULL);

	TwAddVarRW(Bar_HBAO, "Radius", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Radius, NULL);
	TwDefine(" HBAO+/Radius  step=0.01");
	
	TwAddVarRW(Bar_HBAO, "MetersToViewSpaceUnits", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.MetersToViewSpaceUnits, NULL);
	TwDefine(" HBAO+/MetersToViewSpaceUnits  step=0.01");

	TwAddVarRW(Bar_HBAO, "PowerExponent", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.PowerExponent, NULL);
	TwDefine(" HBAO+/PowerExponent  step=0.01");

	TwAddVarRW(Bar_HBAO, "Bias", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Bias, NULL);
	TwDefine(" HBAO+/Bias  step=0.01");

	TwAddVarRW(Bar_HBAO, "BlurSharpness", TW_TYPE_FLOAT, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.BlurSharpness, NULL);
	TwDefine(" HBAO+/BlurSharpness  step=0.01");

	TwType tbm = TwDefineEnumFromString("BlendModeEnum", "0 {Replace}, 1 {Multiply}");
	TwAddVarRW(Bar_HBAO, "BlendMode", tbm, &Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.BlendMode, NULL);

	Bar_ShaderMakros = TwNewBar("ShaderMakros");
	TwDefine(" ShaderMakros position='1200 0'");
	
	TwAddSeparator(Bar_ShaderMakros, "Reload shaders to apply changes!", NULL);
	TwAddVarRW(Bar_ShaderMakros, "Enable Shadows", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows, NULL);
	TwAddVarRW(Bar_ShaderMakros, "Filter shadows", TW_TYPE_BOOLCPP, &Engine::GAPI->GetRendererState()->RendererSettings.EnableSoftShadows, NULL);
	TwAddButton(Bar_ShaderMakros, "Apply", (TwButtonCallback)ReloadShadersButtonCallback, this, NULL); 

	/*Bar_TextureSettings = TwNewBar("TextureSettings");
	TwAddVarRW(Bar_TextureSettings, "Texture", TW_TYPE_CSSTRING(256), TS_TextureName, "");
	TwDefine(" TextureSettings size='455 180' valueswidth=200 position='400 320'");*/

	return XR_SUCCESS;
}

/** On window message */
LRESULT BaseAntTweakBar::OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(IsActive)
		return TwEventWin(hWnd, msg, wParam, lParam);

	return true;
}

/** Sets the bars active and visible */
void BaseAntTweakBar::SetActive(bool value)
{
	IsActive = value;

	if(IsActive)
	{
		//InitTextureSettingsBar();
	}
}

bool BaseAntTweakBar::GetActive()
{
	return IsActive;
}

/** Draws the bars */
void BaseAntTweakBar::Draw()
{
	if(IsActive)
	{
		TwDraw();

		// Check texture name
		/*if(LastFrameActiveMaterialInfo != ActiveMaterialInfo)
		{
			if(ActiveMaterialInfo)
			{
				// Update the bar
				UpdateTextureSettingsBar();
			}

			LastFrameActiveMaterialInfo = ActiveMaterialInfo;
		}

		// Check for change
		if(TS_Active)
		{
			if(ActiveMaterialInfo)
			{
				// Check for a change
				if(memcmp(&TS_OldMaterialInfo, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer, sizeof(MaterialInfo)) != 0)
				{
					TS_FrameTexturesInfos[ActiveMaterialInfo].Info->WriteToFile(TS_FrameTexturesInfos[ActiveMaterialInfo].Name);
					TS_OldMaterialInfo = *TS_FrameTexturesInfos[ActiveMaterialInfo].Info;

					LogInfo() << "Saved MaterialInfo: " << TS_TextureName;
				}
			}

			if(ActiveMaterialInfo && TS_FrameTexturesInfos[ActiveMaterialInfo].Texture)
			{
				const int previewSize = 512;
				const int previewDownscale = 2;
				INT2 pPosition;
				pPosition.x = Engine::GraphicsEngine->GetResolution().x - (previewSize / previewDownscale);
				pPosition.y = 0;

				INT2 pSize = INT2(previewSize / previewDownscale,previewSize / previewDownscale);

				TS_FrameTexturesInfos[ActiveMaterialInfo].Texture->Bind(0);
				Engine::GraphicsEngine->DrawQuad(pPosition, pSize);

				// Do this every frame on this one info to get the changes to the constantbuffer from the anttweakbar
				TS_FrameTexturesInfos[ActiveMaterialInfo].Info->UpdateConstantbuffer();
			}
		}*/
	}
}
 
/** Updates the TS_Bar */
void BaseAntTweakBar::UpdateTextureSettingsBar()
{
		/*TwRemoveVar(Bar_TextureSettings, "NormalmapDepth");
		TwRemoveVar(Bar_TextureSettings, "SpecularIntensity");
		TwRemoveVar(Bar_TextureSettings, "SpecularPower");
		TwRemoveVar(Bar_TextureSettings, "DisplacementFactor");
		TwRemoveVar(Bar_TextureSettings, "TriplanarTextureScale");

		TwAddVarRW(Bar_TextureSettings, "NormalmapDepth", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.NormalmapStrength, NULL);
		TwDefine(" TextureSettings/NormalmapDepth  step=0.01");

		TwAddVarRW(Bar_TextureSettings, "SpecularIntensity", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.SpecularIntensity, NULL);
		TwDefine(" TextureSettings/SpecularIntensity  step=0.01");

		TwAddVarRW(Bar_TextureSettings, "SpecularPower", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.SpecularPower, NULL);
		TwDefine(" TextureSettings/SpecularPower  step=0.1");

		TwAddVarRW(Bar_TextureSettings, "DisplacementFactor", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.DisplacementFactor, NULL);
		TwDefine(" TextureSettings/DisplacementFactor  step=0.01");

		TwAddVarRW(Bar_TextureSettings, "TriplanarTextureScale", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.TextureScale, NULL);
		TwDefine(" TextureSettings/TriplanarTextureScale  step=0.1");

		TwAddVarRW(Bar_TextureSettings, "TriplanarFresnelFactor", TW_TYPE_FLOAT, &TS_FrameTexturesInfos[ActiveMaterialInfo].Info->buffer.FresnelFactor, NULL);
		TwDefine(" TextureSettings/TriplanarFresnelFactor  step=0.01");*/
}

static bool FrameTexEnumCompare(TS_TextureInfo& a, TS_TextureInfo& b)
{
	return std::string(a.Name) < std::string(b.Name);
}

/** Initializes the TextureSettingsBar */
void BaseAntTweakBar::InitTextureSettingsBar()
{
	TwRemoveAllVars(Bar_TextureSettings);
	//TwRemoveVar(Bar_TextureSettings, "FrameTextures");
	TS_FrameTexturesInfos.clear();

	std::vector<TS_TextureInfo> e;

	std::string enumDef = "";

	const std::set<zCTexture *>& frameTextures = Engine::GraphicsEngine->GetFrameTextures();
	if(!frameTextures.size())
		return;

	for(std::set<zCTexture *>::const_iterator it = frameTextures.begin(); it != frameTextures.end(); it++)
	{
		e.push_back(TS_TextureInfo(Engine::GAPI->GetMaterialInfoFrom((*it)), (*it)->GetNameWithoutExt(), (*it)));
	}

	// Sort alphabetically
	std::sort(e.begin(), e.end(), FrameTexEnumCompare);

	int prefTexIdx = 0;
	for(unsigned int i=0;i<e.size();i++)
	{
		// Pointer to int. Since Gothic is 32-Bit this can't run in 64-bit mode anyways.
		enumDef += std::to_string(i) + " {" + e[i].Name + "}";

		TS_FrameTexturesInfos.push_back(TS_TextureInfo(e[i].Info, e[i].Name, e[i].Texture));

		if(e[i].Name == TS_PreferredTexture)
		{
			prefTexIdx = i;
			ActiveMaterialInfo = i;
		}

		if(i != e.size() - 1)
			enumDef += ", ";
	}

	
	enumDef += "";

	// Add the list to the bar
	TwType t = TwDefineEnumFromString("frameTextureList", enumDef.c_str());
	std::string def = std::to_string(prefTexIdx) + " {" + TS_PreferredTexture + "}";
	TwAddVarRW(Bar_TextureSettings, "FrameTextures", t, &ActiveMaterialInfo, NULL);

	TS_Active = true;
}


/** Sets the preferred texture for the texture settings */
void BaseAntTweakBar::SetPreferredTextureForSettings(const std::string& texture)
{
	TS_PreferredTexture = texture;
}

void TW_CALL BaseAntTweakBar::ReloadShadersButtonCallback(void *clientData)
{ 
    Engine::GraphicsEngine->ReloadShaders();
}

void TW_CALL BaseAntTweakBar::SaveZENResourcesCallback(void *clientData)
{ 
    Engine::GAPI->SaveCustomZENResources();
}

void TW_CALL BaseAntTweakBar::LoadZENResourcesCallback(void *clientData)
{ 
    Engine::GAPI->LoadCustomZENResources();
}

/** Called on load ZEN resources */
void TW_CALL BaseAntTweakBar::OpenSettingsCallback(void* clientdata)
{
	Engine::GraphicsEngine->OnUIEvent(BaseGraphicsEngine::EUIEvent::UI_OpenSettings);
}

/** Resizes the anttweakbar */
XRESULT BaseAntTweakBar::OnResize(INT2 newRes)
{
	TwWindowSize(newRes.x, newRes.y);

	return XR_SUCCESS;
}