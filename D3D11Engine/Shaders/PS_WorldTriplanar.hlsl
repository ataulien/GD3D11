//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <FFFog.h>
#include <DS_Defines.h>
#include <Triplanar.h>

cbuffer MI_MaterialInfo : register( b2 )
{
	float MI_SpecularIntensity;
	float MI_SpecularPower;
	float MI_NormalmapStrength;
	float MI_DisplacementFactor;
	
	float MI_TextureScale;
	float MI_FresnelFactor;
	float2 MI_Pad;
}

cbuffer POS_MaterialInfo : register( b3 )
{
	float3 OS_AmbientColor;
	float OS_Pad;
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
	float3 vNormalWS		: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	float3 vWorldPosition	: TEXCOORD6;
	float4 vPosition		: SV_POSITION;
};



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{	
	float scale = 1/1500.0f;
	float3 bump = GetTriPlanarNrmMap(TX_Texture1, TX_Texture1, TX_Texture1, SS_Linear,
										scale * MI_TextureScale,
										MI_NormalmapStrength,
										Input.vWorldPosition, 
										Input.vNormalWS,
										Input.vViewPosition,
										Input.vNormalVS);
	float3 tex = GetTriPlanarTexture(TX_Texture0, TX_Texture0, TX_Texture0, SS_Linear,
										scale * MI_TextureScale, 
										Input.vWorldPosition, 
										Input.vNormalWS).rgb;

	float fresnel = 1.0f - saturate(dot(normalize(-Input.vViewPosition), normalize(bump)));
	
	tex = tex + pow(fresnel, 8.0f) * MI_FresnelFactor;
										
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(tex, Input.vDiffuse.y);
	//output.vDiffuse = 1.0f;	
	//output.vDiffuse.rgb = bump;
	output.vNrm_SI_SP.xy = EncodeNormal(normalize(bump));
	output.vNrm_SI_SP.z = MI_SpecularIntensity;
	output.vNrm_SI_SP.w = MI_SpecularPower;
	return output;
}
