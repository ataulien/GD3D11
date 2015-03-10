//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

#include <AtmosphericScattering.h>

cbuffer CloudBuffer : register( b0 )
{
	float3 C_LightDirection;
	float C_Pad;
	
	float3 C_CloudPosition;
	float C_Pad2;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vViewPosition 	: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color;
	float light = max(0.6f, dot(normalize(Input.vNormalVS), C_LightDirection) * 2.0f + 0.5f);
	
	color.a = 1.0f;
	color.rgb = light;
	
	color.rgb = ApplyAtmosphericScatteringGround(Input.vWorldPosition + C_CloudPosition, color.rgb);
	//color.rgb = AC_Wavelength;
	return color;
}














