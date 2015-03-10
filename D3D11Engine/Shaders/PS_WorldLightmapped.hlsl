//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <AtmosphericScattering.h>
#include <FFFog.h>

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Texture1 : register( t1 );



//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

float4 retF(float f)
{
	return float4(f,f,f,1);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 lightmap = TX_Texture1.Sample(SS_Linear, Input.vTexcoord2);
	float4 color = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	
	//return float4(lightmap.rgb, 1);
	
	// Apply static lighting
	color *= lightmap;
	
	// WorldMesh can always do the alphatest
	DoAlphaTest(color.a);
	
	color.rgb = ApplyAtmosphericScatteringGround(Input.vWorldPosition, color.rgb);
	
	return float4(color.rgb, 1);
}

