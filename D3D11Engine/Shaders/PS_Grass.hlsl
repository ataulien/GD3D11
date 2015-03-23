//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

cbuffer MI_MaterialInfo : register( b0 )
{
	float MI_SpecularIntensity;
	float MI_SpecularPower;
	float MI_NormalmapStrength;
	float MI_ParallaxOcclusionStrength;
}


//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
Texture2D	TX_Texture0 : register( t1 );
Texture2D	TX_Ground : register( t0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float3 vNormalVS		: TEXCOORD1;
	float3 vWorldPosition	: TEXCOORD2;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	
	color.rgb = lerp(saturate(dot(float3(0.333f, 0.333f, 0.333f), color.rgb) * 2.0f), color.rgb, 0.5f);
	
	clip(color.a * 0.7f - 0.5f);
	
	color.rgb *= TX_Ground.SampleLevel(SS_Linear, frac(Input.vWorldPosition.xz / 1000), 5) * 1.1f;
	
	//color.rgb *= 1 - pow((Input.vPosition.z / 1.3f), 2.0f);
	
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, 1);
		
	output.vNrm_SI_SP.xy = EncodeNormal(normalize(Input.vNormalVS));
	output.vNrm_SI_SP.z = 0;
	output.vNrm_SI_SP.w = 0;
	return output;
}
