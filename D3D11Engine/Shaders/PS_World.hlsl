//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
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

/*cbuffer POS_MaterialInfo : register( b3 )
{
	float3 OS_AmbientColor;
	float OS_Pad;
}*/

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
	
	ClipDistanceEffect(Input.vViewPosition.z, DIST_DrawDistance, color.r * 2 - 1, 500.0f);
	
	// WorldMesh can always do the alphatest
	DoAlphaTest(color.a);


	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, Input.vDiffuse.y);
		

	output.vNrm_SI_SP.xy = EncodeNormal(normalize(Input.vNormalVS));
	output.vNrm_SI_SP.z = MI_SpecularIntensity;
	output.vNrm_SI_SP.w = MI_SpecularPower;
	return output;
}
