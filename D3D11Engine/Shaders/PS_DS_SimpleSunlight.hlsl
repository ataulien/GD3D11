//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

#include <AtmosphericScattering.h>

static const float EMISSIVE_COEFFICIENT = 39.78f;

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
Texture2D	TX_Diffuse : register( t0 );
Texture2D	TX_Nrm_SI_SP : register( t1 );
Texture2D	TX_Depth : register( t2 );
Texture2D	TX_Shadowmap : register( t3 );




//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexCoord 		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

float3 VSPositionFromDepth(float depth, float2 vTexCoord)
{
    // Get the depth value for this pixel
    float z = depth; 
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    // Transform by the inverse projection matrix
    float4 vPositionVS = mul(vProjectedPos, SQ_InvProj); //invViewProj == invProjection here  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;   
}

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
	
	// Look up the diffuse color
	float4 diffuse = TX_Diffuse.Sample(SS_Linear, uv);
	float vertLighting = diffuse.a;
	
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
	float sunStrength = dot(SQ_LightColor.rgb, float3(0.333f,0.333f,0.333f));
	float3 sun = saturate(dot(normalize(SQ_LightDirectionVS), normal)) * 1.0f;
	diffuse.rgb = lerp(diffuse * 0.35 * (sunStrength), diffuse * (SQ_LightColor), sun);
	
	// Fix indoor stuff
	diffuse *= vertLighting;
	//diffuse.rgb = lerp(diffuse.rgb, 1.0f, clamp(shaft, 0.0f, 0.4f));
	
	//return float4(pow(spec, specPower) * specIntensity.xxx * diffuse.rgb * SQ_LightColor.rgb,1);
	return float4(diffuse.rgb,1);
}

