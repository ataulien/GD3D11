//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Diffuse : register( t0 );
Texture2D	TX_Normal : register( t1 );
Texture2D	TX_Depth : register( t2 );
TextureCube	TX_ReflectionCube : register( t3 );

cbuffer WettnessConstantBuffer : register( b0 )
{
	matrix W_InvProj;
	float W_Wettness;
	float3 W_Pad1;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
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
    float4 vPositionVS = mul(vProjectedPos, W_InvProj); //invViewProj == invProjection here  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;   
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color = TX_Diffuse.Sample(SS_Linear, Input.vTexcoord);
	
	// Reconstruct VS World Position from depth
	/*float expDepth = TX_Depth.Sample(SS_Linear, uv).r;
	float3 vsPosition = VSPositionFromDepth(expDepth, uv);
	float3 V = normalize(-vsPosition);
	
	// Decode the view-space normal back
	float3 normal = normalize(DecodeNormal(gb2.xy));	
	
	float fresnel = pow(1.0f - max(0.0f, dot(normal, V)), 10.0f);
	litPixel += lerp(fresnel * litPixel * 0.5f, 0.0f, sun);
	
	// Reflection
	float3 reflect_vec = reflect(-viewDirection, wavesFres);
	
	// sample reflection cube
	float3 reflection = TX_ReflectionCube.Sample(SS_Linear, reflect_vec).xyz;*/
	
	color = lerp(color * float4(1,0,0,1), color, W_Wettness);
	
	return color;
}

