//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <AtmosphericScattering.h>
#include <FFFog.h>
#include <DS_Defines.h>
#include <Toolbox.h>

cbuffer MI_MaterialInfo : register( b2 )
{
	float MI_SpecularIntensity;
	float MI_SpecularPower;
	float MI_NormalmapStrength;
	float MI_ParallaxOcclusionStrength;
}

cbuffer DIST_Distance : register( b3 )
{
	float DIST_DrawDistance;
	float3 DIST_Pad;
}

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Texture1 : register( t1 );
Texture2D	TX_Texture2 : register( t2 );
TextureCube	TX_ReflectionCube : register( t4 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	
	
	// Do alphatest if wanted
#if ALPHATEST == 1
	ClipDistanceEffect(length(Input.vViewPosition), DIST_DrawDistance, color.r * 2 - 1, 500.0f);
	
	// WorldMesh can always do the alphatest
	DoAlphaTest(color.a);
#endif



	// Apply normalmapping if wanted
#if NORMALMAPPING == 1
	float3 nrm = perturb_normal(Input.vNormalVS, Input.vViewPosition, TX_Texture1, Input.vTexcoord, SS_Linear, MI_NormalmapStrength);
#else
	float3 nrm = normalize(Input.vNormalVS);
#endif

	float4 fx;
#if FXMAP == 1
	fx = TX_Texture2.Sample(SS_Linear, Input.vTexcoord);
#else
	fx = 1.0f;
#endif
		
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, Input.vDiffuse.y);
	//output.vDiffuse = float4(Input.vTexcoord2, 0, 1);
	//output.vDiffuse = float4(Input.vNormalVS, 1);
	
	output.vNrm_SI_SP.xy = EncodeNormal(nrm);
	output.vNrm_SI_SP.z = MI_SpecularIntensity * fx.r;
	output.vNrm_SI_SP.w = MI_SpecularPower * fx.g;
	return output;
}

