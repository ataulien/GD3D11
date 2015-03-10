//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <GaussBlur.h>

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Depth : register( t1 );

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
	float depth = TX_Depth.Sample(SS_Linear, Input.vTexcoord);

	float2 ps = float2(1.0f / 1920.0f, 1.0f / 1080.0f);
	ps = lerp(float2(0,0), ps, pow(depth, 300));
	
	float4 blur = DoBlurPassSingle(ps, Input.vTexcoord, TX_Texture0, TX_Depth, SS_Linear, 1.0f);
	
	float4 scene = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	
	blur.a = 1.0f;
	scene.a = 1.0f;

	return blur;
}

