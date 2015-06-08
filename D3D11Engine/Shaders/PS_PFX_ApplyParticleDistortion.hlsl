//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Scene : register( t0 );
Texture2D	TX_ParticleColor : register( t1 );
Texture2D	TX_ParticleDistortion : register( t2 );

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
	
	const float maxDistortion = 0.08f;
	
	float4 color = TX_ParticleColor.Sample(SS_Linear, uv);
	float2 distortion = TX_ParticleDistortion.Sample(SS_Linear, uv).xy;
	distortion = min(float2(maxDistortion, maxDistortion), distortion);
	distortion = max(float2(-maxDistortion, -maxDistortion), distortion);
	
	float4 scene = TX_Scene.Sample(SS_Linear, saturate(uv + distortion));
	
	// Alpha is just the luminance of the rendered particles
	float alpha = dot(color.rgb, float3(0.333f, 0.333f, 0.333f)) * 2.0f;
	
	//return float4(saturate(+ distortion),0,1);
	//return float4(color.aaa, 1);
	
	return float4(scene + color.rgb, 1);
}

