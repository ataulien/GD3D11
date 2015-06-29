#include "pch.h"
#include "D3D11ShaderManager.h"
#include "D3D11Vshader.h"
#include "D3D11PShader.h"
#include "D3D11HDShader.h"
#include "D3D11GShader.h"
#include "D3D11ConstantBuffer.h"
#include "GothicGraphicsState.h"
#include "ConstantBufferStructs.h"
#include "GothicAPI.h"
#include "Engine.h"

const int NUM_MAX_BONES = 96;

D3D11ShaderManager::D3D11ShaderManager()
{
	ReloadShadersNextFrame = false;
}

D3D11ShaderManager::~D3D11ShaderManager()
{
	DeleteShaders();
}

/** Creates list with ShaderInfos */
XRESULT D3D11ShaderManager::Init()
{
	Shaders = std::vector<ShaderInfo>();
	VShaders = std::unordered_map<std::string, D3D11VShader*>();
	PShaders = std::unordered_map<std::string, D3D11PShader*>();

	Shaders.push_back(ShaderInfo("VS_Ex", "VS_Ex.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_ExCube", "VS_ExCube.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));
	
	Shaders.push_back(ShaderInfo("VS_PNAEN", "VS_PNAEN.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_PNAEN_Instanced", "VS_PNAEN_Instanced.hlsl", "v", 10));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_Decal", "VS_Decal.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_ExWater", "VS_ExWater.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_ParticlePoint", "VS_ParticlePoint.hlsl", "v", 11));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));

	Shaders.push_back(ShaderInfo("VS_ParticlePointShaded", "VS_ParticlePointShaded.hlsl", "v", 11));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(ParticlePointShadingConstantBuffer));
	

	Shaders.push_back(ShaderInfo("VS_AdvanceRain", "VS_AdvanceRain.hlsl", "v", 11));
	Shaders.back().cBufferSizes.push_back(sizeof(AdvanceRainConstantBuffer));

	Shaders.push_back(ShaderInfo("VS_Ocean", "VS_Ocean.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_ExWS", "VS_ExWS.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_ExDisplace", "VS_ExDisplace.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("VS_Obj", "VS_Obj.hlsl", "v", 8));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));
	
	Shaders.push_back(ShaderInfo("VS_ExSkeletal", "VS_ExSkeletal.hlsl", "v", 3));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstanceSkeletal));
	Shaders.back().cBufferSizes.push_back(NUM_MAX_BONES * sizeof(D3DXMATRIX));

	Shaders.push_back(ShaderInfo("VS_ExSkeletalCube", "VS_ExSkeletalCube.hlsl", "v", 3));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstanceSkeletal));
	Shaders.back().cBufferSizes.push_back(NUM_MAX_BONES * sizeof(D3DXMATRIX));

	
	Shaders.push_back(ShaderInfo("VS_PNAEN_Skeletal", "VS_PNAEN_Skeletal.hlsl", "v", 3));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstanceSkeletal));
	Shaders.back().cBufferSizes.push_back(NUM_MAX_BONES * sizeof(D3DXMATRIX));

	Shaders.push_back(ShaderInfo("VS_TransformedEx", "VS_TransformedEx.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(2 * sizeof(float2));

	Shaders.push_back(ShaderInfo("VS_ExPointLight", "VS_ExPointLight.hlsl", "v", 1));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_PointLightConstantBuffer));

	Shaders.push_back(ShaderInfo("VS_XYZRHW_DIF_T1", "VS_XYZRHW_DIF_T1.hlsl", "v", 7));
	Shaders.back().cBufferSizes.push_back(2 * sizeof(float2));

	Shaders.push_back(ShaderInfo("VS_ExInstancedObj", "VS_ExInstancedObj.hlsl", "v", 10));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));

	Shaders.push_back(ShaderInfo("VS_ExInstanced", "VS_ExInstanced.hlsl", "v", 4));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(GrassConstantBuffer));

	Shaders.push_back(ShaderInfo("VS_GrassInstanced", "VS_GrassInstanced.hlsl", "v", 9));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));

	Shaders.push_back(ShaderInfo("VS_Lines", "VS_Lines.hlsl", "v", 6));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerFrame));
	Shaders.back().cBufferSizes.push_back(sizeof(VS_ExConstantBuffer_PerInstance));

	Shaders.push_back(ShaderInfo("PS_Lines", "PS_Lines.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_LinesSel", "PS_LinesSel.hlsl", "p"));

	//Shaders.push_back(ShaderInfo("FixedFunctionPipelineEmulationPS", "FixedFunctionPipelineEmulationPS.hlsl", "p", 1));
	Shaders.push_back(ShaderInfo("PS_Simple", "PS_Simple.hlsl", "p"));

	Shaders.push_back(ShaderInfo("PS_Rain", "PS_Rain.hlsl", "p"));

	

	Shaders.push_back(ShaderInfo("PS_World", "PS_World.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	Shaders.push_back(ShaderInfo("PS_Ocean", "PS_Ocean.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(OceanSettingsConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(OceanPerPatchConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(RefractionInfoConstantBuffer));
	

	Shaders.push_back(ShaderInfo("PS_Water", "PS_Water.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(RefractionInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_ParticleDistortion", "PS_ParticleDistortion.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(RefractionInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_ApplyParticleDistortion", "PS_PFX_ApplyParticleDistortion.hlsl", "p"));

	Shaders.push_back(ShaderInfo("PS_WorldTriplanar", "PS_WorldTriplanar.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	Shaders.push_back(ShaderInfo("PS_Grass", "PS_Grass.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));

	Shaders.push_back(ShaderInfo("PS_Sky", "PS_Sky.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(SkyConstantBuffer));

	Shaders.push_back(ShaderInfo("VS_PFX", "VS_PFX.hlsl", "v", 5));
	Shaders.back().cBufferSizes.push_back(sizeof(PFXVS_ConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_Simple", "PS_PFX_Simple.hlsl", "p"));


	Shaders.push_back(ShaderInfo("PS_PFX_GaussBlur", "PS_PFX_GaussBlur.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(BlurConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_Heightfog", "PS_PFX_Heightfog.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(HeightfogConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_UnderwaterFinal", "PS_PFX_UnderwaterFinal.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(RefractionInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_Cloud", "PS_Cloud.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(CloudConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_Copy_NoAlpha", "PS_PFX_Copy_NoAlpha.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_PFX_Blend", "PS_PFX_Blend.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_PFX_DistanceBlur", "PS_PFX_DistanceBlur.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_PFX_LumConvert", "PS_PFX_LumConvert.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_PFX_LumAdapt", "PS_PFX_LumAdapt.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(LumAdaptConstantBuffer));
	Shaders.push_back(ShaderInfo("PS_PFX_HDR", "PS_PFX_HDR.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(HDRSettingsConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_GodRayMask", "PS_PFX_GodRayMask.hlsl", "p"));
	Shaders.push_back(ShaderInfo("PS_PFX_GodRayZoom", "PS_PFX_GodRayZoom.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GodRayZoomConstantBuffer));
	
	Shaders.push_back(ShaderInfo("PS_PFX_Tonemap", "PS_PFX_Tonemap.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(HDRSettingsConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_SkyPlane", "PS_SkyPlane.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(ViewportInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_AtmosphereGround", "PS_AtmosphereGround.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	Shaders.push_back(ShaderInfo("PS_Atmosphere", "PS_Atmosphere.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_AtmosphereOuter", "PS_AtmosphereOuter.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_WorldLightmapped", "PS_WorldLightmapped.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));

	Shaders.push_back(ShaderInfo("PS_FixedFunctionPipe", "PS_FixedFunctionPipe.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));

	Shaders.push_back(ShaderInfo("PS_DS_PointLight", "PS_DS_PointLight.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_PointLightConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_DS_PointLightDynShadow", "PS_DS_PointLightDynShadow.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_PointLightConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_DS_AtmosphericScattering", "PS_DS_AtmosphericScattering.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_ScreenQuadConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_DS_SimpleSunlight", "PS_DS_SimpleSunlight.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_ScreenQuadConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	
	Shaders.push_back(ShaderInfo("DefaultTess", "DefaultTess.hlsl", "hd"));
	Shaders.back().cBufferSizes.push_back(sizeof(DefaultHullShaderConstantBuffer));

	Shaders.push_back(ShaderInfo("OceanTess", "OceanTess.hlsl", "hd"));
	Shaders.back().cBufferSizes.push_back(sizeof(DefaultHullShaderConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(OceanSettingsConstantBuffer));
	
	Shaders.push_back(ShaderInfo("PNAEN_Tesselation", "PNAEN_Tesselation.hlsl", "hd"));
	Shaders.back().cBufferSizes.push_back(sizeof(PNAENConstantBuffer));

	Shaders.push_back(ShaderInfo("GS_Billboard", "GS_Billboard.hlsl", "g"));
	Shaders.back().cBufferSizes.push_back(sizeof(ParticleGSInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("GS_Raindrops", "GS_Raindrops.hlsl", "g"));
	Shaders.back().cBufferSizes.push_back(sizeof(ParticleGSInfoConstantBuffer));

	Shaders.push_back(ShaderInfo("GS_Cubemap", "GS_Cubemap.hlsl", "g"));
	Shaders.back().cBufferSizes.push_back(sizeof(CubemapGSConstantBuffer));

	Shaders.push_back(ShaderInfo("GS_ParticleStreamOut", "VS_AdvanceRain.hlsl", "g", 11));
	Shaders.back().cBufferSizes.push_back(sizeof(ParticleGSInfoConstantBuffer));

	D3D10_SHADER_MACRO m;
	std::vector<D3D10_SHADER_MACRO> makros;

	m.Name = "NORMALMAPPING";
	m.Definition = "0";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "0";
	makros.push_back(m);

	Shaders.push_back(ShaderInfo("PS_Diffuse", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	makros.clear();

	m.Name = "APPLY_RAIN_EFFECTS";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "SHD_ENABLE";
	m.Definition = "1";
	makros.push_back(m);

	Shaders.push_back(ShaderInfo("PS_DS_AtmosphericScattering_Rain", "PS_DS_AtmosphericScattering.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(DS_ScreenQuadConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));

	makros.clear();

	Shaders.push_back(ShaderInfo("PS_LinDepth", "PS_LinDepth.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	
	m.Name = "NORMALMAPPING";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "0";
	makros.push_back(m);

	Shaders.push_back(ShaderInfo("PS_DiffuseNormalmapped", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	makros.clear();
	m.Name = "NORMALMAPPING";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "0";
	makros.push_back(m);

	m.Name = "FXMAP";
	m.Definition = "1";
	makros.push_back(m);

	Shaders.push_back(ShaderInfo("PS_DiffuseNormalmappedFxMap", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	makros.clear();
	m.Name = "NORMALMAPPING";
	m.Definition = "0";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "1";
	makros.push_back(m);

	Shaders.push_back(ShaderInfo("PS_DiffuseAlphaTest", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	makros.clear();
	m.Name = "NORMALMAPPING";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "1";
	makros.push_back(m);
								  
	Shaders.push_back(ShaderInfo("PS_DiffuseNormalmappedAlphaTest", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));

	makros.clear();
	m.Name = "NORMALMAPPING";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "FXMAP";
	m.Definition = "1";
	makros.push_back(m);
								  
	Shaders.push_back(ShaderInfo("PS_DiffuseNormalmappedAlphaTestFxMap", "PS_Diffuse.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));


	

	makros.clear();
	m.Name = "RENDERMODE";
	m.Definition = "0";
	makros.push_back(m);
	Shaders.push_back(ShaderInfo("PS_Preview_White", "PS_Preview.hlsl", "p", makros));

	makros.clear();
	m.Name = "RENDERMODE";
	m.Definition = "1";
	makros.push_back(m);
	Shaders.push_back(ShaderInfo("PS_Preview_Textured", "PS_Preview.hlsl", "p", makros));

	makros.clear();
	m.Name = "RENDERMODE";
	m.Definition = "2";
	makros.push_back(m);
	Shaders.push_back(ShaderInfo("PS_Preview_TexturedLit", "PS_Preview.hlsl", "p", makros));

	makros.clear();

	Shaders.push_back(ShaderInfo("PS_PFX_Sharpen", "PS_PFX_Sharpen.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GammaCorrectConstantBuffer));

	Shaders.push_back(ShaderInfo("PS_PFX_GammaCorrectInv", "PS_PFX_GammaCorrectInv.hlsl", "p"));
	Shaders.back().cBufferSizes.push_back(sizeof(GammaCorrectConstantBuffer));

	
	// --- LPP
	makros.clear();
	m.Name = "NORMALMAPPING";
	m.Definition = "1";
	makros.push_back(m);

	m.Name = "ALPHATEST";
	m.Definition = "1";
	makros.push_back(m);
								  
	Shaders.push_back(ShaderInfo("PS_LPPNormalmappedAlphaTest", "PS_LPP.hlsl", "p", makros));
	Shaders.back().cBufferSizes.push_back(sizeof(GothicGraphicsState));
	Shaders.back().cBufferSizes.push_back(sizeof(AtmosphereConstantBuffer));
	Shaders.back().cBufferSizes.push_back(sizeof(MaterialInfo::Buffer));
	Shaders.back().cBufferSizes.push_back(sizeof(PerObjectState));




	return XR_SUCCESS;
}

/** Loads/Compiles Shaderes from list */
XRESULT D3D11ShaderManager::LoadShaders()
{
	for (unsigned int i = 0; i < Shaders.size(); i++)
	{
		//Check if shader src-file exists
		std::string fileName = Engine::GAPI->GetStartDirectory() + "\\system\\GD3D11\\shaders\\" + Shaders[i].fileName;
		if (FILE* f = fopen(fileName.c_str(), "r"))
		{
			//Check shader's type
			if (Shaders[i].type == "v")
			{
				// See if this is a reload
				if(VShaders.count(Shaders[i].name) > 0)
				{
					LogInfo() << "Reloading shader: " << Shaders[i].name;

					D3D11VShader* vs = new D3D11VShader();
					if(XR_SUCCESS != vs->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].layout, Shaders[i].shaderMakros))
					{
						LogError() << "Failed to reload shader: " << Shaders[i].fileName;

						delete vs;
					}else
					{
						// Compilation succeeded, switch the shader
						delete VShaders[Shaders[i].name];
						VShaders[Shaders[i].name] = vs;

						for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
						{
							VShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
						}
					}
				}else
				{
					LogInfo() << "Reloading shader: " << Shaders[i].name;

					VShaders[Shaders[i].name] = new D3D11VShader();
					XLE(VShaders[Shaders[i].name]->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].layout, Shaders[i].shaderMakros));
					for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
					{
						VShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
					}
				}
			}
			else if (Shaders[i].type == "p")
			{
				// See if this is a reload
				if(PShaders.count(Shaders[i].name) > 0)
				{
					D3D11PShader* ps = new D3D11PShader();
					if(XR_SUCCESS != ps->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].shaderMakros))
					{
						LogError() << "Failed to reload shader: " << Shaders[i].fileName;

						delete ps;
					}else
					{
						// Compilation succeeded, switch the shader
						delete PShaders[Shaders[i].name];
						PShaders[Shaders[i].name] = ps;

						for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
						{
							PShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
						}
					}
				}else
				{
					PShaders[Shaders[i].name] = new D3D11PShader();
					XLE(PShaders[Shaders[i].name]->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].shaderMakros));
					for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
					{
						PShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
					}
				}
			}else if (Shaders[i].type == "g")
			{
				// See if this is a reload
				if(GShaders.count(Shaders[i].name) > 0)
				{
					D3D11GShader* gs = new D3D11GShader();
					if(XR_SUCCESS != gs->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].shaderMakros, Shaders[i].layout != 0, Shaders[i].layout))
					{
						LogError() << "Failed to reload shader: " << Shaders[i].fileName;

						delete gs;
					}else
					{
						// Compilation succeeded, switch the shader
						delete GShaders[Shaders[i].name];
						GShaders[Shaders[i].name] = gs;

						for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
						{
							GShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
						}
					}
				}else
				{
					GShaders[Shaders[i].name] = new D3D11GShader();
					XLE(GShaders[Shaders[i].name]->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), Shaders[i].shaderMakros, Shaders[i].layout != 0, Shaders[i].layout));
					for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
					{
						GShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
					}
				}
			}

			fclose(f);
		}

		// Hull/Domain shaders are handled differently, they check inside for missing file
		if (Shaders[i].type == std::string("hd"))
		{
			// See if this is a reload
			if(HDShaders.count(Shaders[i].name) > 0)
			{
				D3D11HDShader* hds = new D3D11HDShader();
				if(XR_SUCCESS != hds->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), 
					("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str()))
				{
					LogError() << "Failed to reload shader: " << Shaders[i].fileName;

					delete hds;
				}else
				{
					// Compilation succeeded, switch the shader
					delete HDShaders[Shaders[i].name];
					HDShaders[Shaders[i].name] = hds;

					for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
					{
						HDShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
					}
				}
			}else
			{
				HDShaders[Shaders[i].name] = new D3D11HDShader();
				XLE(HDShaders[Shaders[i].name]->LoadShader(("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str(), 
					("system\\GD3D11\\shaders\\" + Shaders[i].fileName).c_str()));
				for (unsigned int j = 0; j < Shaders[i].cBufferSizes.size(); j++)
				{
					HDShaders[Shaders[i].name]->GetConstantBuffer().push_back(new D3D11ConstantBuffer(Shaders[i].cBufferSizes[j], NULL));
				}
			}
		}

	}

	return XR_SUCCESS;
}

/** Deletes all shaders and loads them again */
XRESULT D3D11ShaderManager::ReloadShaders()
{
	ReloadShadersNextFrame = true;

	return XR_SUCCESS;
}

/** Called on frame start */
XRESULT D3D11ShaderManager::OnFrameStart()
{
	if(ReloadShadersNextFrame)
	{
		LoadShaders();
		ReloadShadersNextFrame = false;
	}

	return XR_SUCCESS;
}

/** Deletes all shaders */
XRESULT D3D11ShaderManager::DeleteShaders()
{
	std::unordered_map<std::string, D3D11VShader*>::iterator vIter;
	for (vIter = VShaders.begin(); vIter != VShaders.end(); vIter++)
	{
		delete vIter->second;
	}
	VShaders.clear();

	std::unordered_map<std::string, D3D11PShader*>::iterator pIter;
	for (pIter = PShaders.begin(); pIter != PShaders.end(); pIter++)
	{
		delete pIter->second;
	}
	PShaders.clear();

	std::unordered_map<std::string, D3D11HDShader*>::iterator hdIter;
	for (hdIter = HDShaders.begin(); hdIter != HDShaders.end(); hdIter++)
	{
		delete hdIter->second;
	}
	HDShaders.clear();

	return XR_SUCCESS;
}

/** Return a specific shader */
D3D11VShader* D3D11ShaderManager::GetVShader(std::string shader)
{
	return VShaders[shader];
}
D3D11PShader* D3D11ShaderManager::GetPShader(std::string shader)
{
	return PShaders[shader];
}

D3D11HDShader* D3D11ShaderManager::GetHDShader(std::string shader)
{
	return HDShaders[shader];
}


D3D11GShader* D3D11ShaderManager::GetGShader(std::string shader)
{
	return GShaders[shader];
}