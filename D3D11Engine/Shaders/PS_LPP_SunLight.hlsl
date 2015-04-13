//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

cbuffer DS_ScreenQuadConstantBuffer : register( b0 )
{
	matrix SQ_InvProj; // Optimize out!
	matrix SQ_InvView;
	float3 SQ_LightDirectionVS;
	float SQ_ShadowmapSize;
	
	float4 SQ_LightColor;
	matrix SQ_ShadowView;
	matrix SQ_ShadowProj;
	
	float2 ProjAB;
	float2 SQ_Pad2;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
SamplerComparisonState SS_Comp : register( s2 );
Texture2D	TX_Nrm_SI_SP : register( t1 );
Texture2D	TX_Depth : register( t2 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexCoord 		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

float3 VSPositionFromDepthFast(float depth, float3 viewRay)
{
	float linearDepth = ProjAB.y / (depth - ProjAB.x);
	
	return viewRay * linearDepth;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	// Get screen UV
	float2 uv = Input.vTexCoord; 
	
	// Get the second GBuffer
	float4 gb2 = TX_Nrm_SI_SP.Sample(SS_Linear, uv);
	
	// Decode the view-space normal back
	float3 normal = DecodeNormal(gb2.xy);
	normal = normalize(normal);
	
	// Get specular parameters
	float specIntensity = gb2.z;
	float specPower = gb2.w;
	
	// Reconstruct VS World Position from depth
	float expDepth = TX_Depth.Sample(SS_Linear, uv).r;
	float3 vsPosition = VSPositionFromDepthFast(expDepth, Input.vEyeRay);

	// Apply sunlight
	float3 sun = max(0, dot(SQ_LightDirectionVS, normal));
	
	return float4(sun.rgb,1);
}

