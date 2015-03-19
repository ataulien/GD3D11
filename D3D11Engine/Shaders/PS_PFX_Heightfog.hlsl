//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

#include <AtmosphericScattering.h>

cbuffer PFXBuffer : register( b0 )
{
	matrix HF_InvProj;
	matrix HF_InvView;
	float3 HF_CameraPosition;
	float HF_FogHeight;

	float HF_HeightFalloff;
	float HF_GlobalDensity;
	float HF_WeightZNear;
	float HF_WeightZFar;

	float3 HF_FogColorMod;
	float HF_pad2;

	float2 HF_ProjAB;
	float2 HF_Pad3;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Depth : register( t1 );

float3 VSPositionFromDepth(float depth, float2 vTexCoord)
{
    // Get the depth value for this pixel
    float z = depth; 
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    // Transform by the inverse projection matrix
    float4 vPositionVS = mul(vProjectedPos, HF_InvProj); //invViewProj == invProjection here  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;   
}

float ComputeVolumetricFog(float3 cameraToWorldPos, float3 posOriginal)
{	
	float cVolFogHeightDensityAtViewer = exp( -HF_HeightFalloff * HF_CameraPosition.y );
	
	float lenOrig = length(posOriginal - HF_CameraPosition);
	float len = length(cameraToWorldPos);
	float fogInt = len * cVolFogHeightDensityAtViewer;
	const float	cSlopeThreshold = 0.01;
	
	float w = saturate((lenOrig-HF_WeightZNear)/(HF_WeightZFar-HF_WeightZNear));

	if(abs( cameraToWorldPos.y ) > cSlopeThreshold )
	{
		float t = HF_HeightFalloff * cameraToWorldPos.y * w;
		fogInt *= (	1.0	- exp( -t ) ) / t;
		
	}
	
	
	
	return	exp( -HF_GlobalDensity * w * fogInt);
}

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float expDepth = TX_Depth.Sample(SS_Linear, Input.vTexcoord).r;
	
	float3 position = VSPositionFromDepth(expDepth, Input.vTexcoord);
	
	
	position = mul(float4(position, 1), HF_InvView).xyz;
	float3 posOriginal = position;
	
	position -= HF_CameraPosition;
	
	position.y -= HF_FogHeight;
	
	float fog = 1.0f - ComputeVolumetricFog(position, posOriginal);
	
	
	
	float3 color = ApplyAtmosphericScatteringGround(position, HF_FogColorMod, false);
	
	return float4(saturate(color), saturate(fog));
	
	//float zView = HF_ProjAB.y / (expDepth - HF_ProjAB.x);
	
	//return float4(HF_CameraPosition + Input.vEyeRay * zView, 1);// fog);
}

