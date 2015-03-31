//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

cbuffer RefractionInfo : register( b3 )
{
	float4x4 RI_Projection;
	float2 RI_ViewportSize;
	float RI_Time;
	float RI_Pad1;
	
	float3 RI_CameraPosition;
	float RI_Pad2;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Distortion : register( t2 );
Texture2D	TX_Depth : register( t3 );

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
	float2 uv = Input.vTexcoord;
	float depth = TX_Distortion.Sample(SS_Linear, uv).r;
	
	// Sample two distortion vectors, modified by time and scaled by a sine-curve
	uv += (TX_Distortion.Sample(SS_Linear, 0.2f * Input.vTexcoord + RI_Time * 0.005f) * 2 - 1 ) * 0.004f;
	uv += (TX_Distortion.Sample(SS_Linear, 0.1f * Input.vTexcoord * float2(-0.7, 0.8) + RI_Time * 0.01f) * 2 - 1) * 0.006f;
	
	uv = saturate(uv);
	
	uv = lerp(uv, Input.vTexcoord, depth / 500.0f);
	
	float4 color = TX_Texture0.Sample(SS_Linear, uv);
	//color *= float4(1,0,0,1);
	
	return color.rgba;
}

