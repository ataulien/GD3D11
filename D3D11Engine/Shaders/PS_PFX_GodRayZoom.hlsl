//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

cbuffer GodRayZoomConstantBuffer : register( b0 )
{
	float GR_Decay;
	float GR_Weight;
	float2 GR_Center;
	
	float GR_Density;
	float3 GR_ColorMod;
};

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
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	const int NUM_SAMPLES = 30;
	float2 center = GR_Center;
	float zoomMax = 0.5f;
	float3 color = 0;
	float illumDecay = 1.0f;
	
	float2 deltaTexCoord = Input.vTexcoord - center;
	deltaTexCoord *= 1.0f / NUM_SAMPLES * GR_Density;
	
	float2 uv = Input.vTexcoord;
	
	[unroll]
	for(int i=0;i<NUM_SAMPLES;i++)
	{
		uv -= deltaTexCoord;
		color += pow(TX_Texture0.Sample(SS_Linear, uv), 1) * illumDecay * GR_Weight;
		
		illumDecay *= GR_Decay;
	}
	color /= NUM_SAMPLES;

	return float4(color * GR_ColorMod,1);
}

