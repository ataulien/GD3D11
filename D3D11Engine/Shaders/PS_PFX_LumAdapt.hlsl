//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

cbuffer LumConvertCB : register( b0 )
{
	float LC_DeltaTime;
	float3 LC_Pad;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_LumLast : register( t1 );
Texture2D	TX_LumCurrent : register( t2 );

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
float PSMain( PS_INPUT Input ) : SV_TARGET
{
	float fLastLum = TX_LumLast.SampleLevel(SS_Linear, float2(0.5f, 0.5f), 9).r;
	float fCurrentLum = TX_LumCurrent.SampleLevel(SS_Linear, float2(0.5f, 0.5f), 9).r;

	// Adapt the luminance using Pattanaik's technique
	const float fTau = 0.5f;
	float fAdaptedLum = fLastLum + (fCurrentLum - fLastLum) * (1 - exp(-LC_DeltaTime * fTau));
	
	return clamp(fAdaptedLum, 0, 32.0f); 
}

