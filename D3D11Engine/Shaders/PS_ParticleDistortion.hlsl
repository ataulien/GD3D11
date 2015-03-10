//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

cbuffer RefractionInfo : register( b0 )
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
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Scene : register( t2 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 c = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	float dist = dot(float3(0.333f,0.333f,0.333f), c.rgb) * c.a;
	//dist = dist > 0.1f ? dist : 0;
	dist = dist * 0.02f * Input.vDiffuse;
	float3 color = TX_Scene.Sample(SS_Linear, (Input.vPosition.xy / RI_ViewportSize) + dist).rgb;
	
	return float4(color, 1);
}

