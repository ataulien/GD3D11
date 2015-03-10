//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

cbuffer vpi : register( b0 )
{
	float2 VPI_ViewportSize;
	float2 VPI_pad;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Texture1 : register( t1 );
Texture2D	TX_Texture2 : register( t2 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vViewPosition 	: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	float2 uv = Input.vPosition.xy / VPI_ViewportSize.xy;	
	
	// Big distortion
	float distortion = TX_Texture1.Sample(SS_Linear, (Input.vTexcoord / 5.0f)).x * 2 - 1;
	distortion /= (Input.vViewPosition.z);
	distortion *= 80.0f;
	
	//return float4(distortion,distortion,distortion, 1);
	
	// Smaller distortion
	float distortion2 = TX_Texture1.Sample(SS_Linear, (Input.vTexcoord / 10.0f)).x * 2 - 1;
	distortion2 /= (Input.vViewPosition.z);
	distortion2 *= 120.0f;
	
	//distortion += float2(-distortion2, distortion2);
	
	float4 cBC = TX_Texture2.Sample(SS_Linear, uv + distortion);
	float4 cNBC = TX_Texture2.Sample(SS_Linear, uv + distortion2);
	
	float4 color = lerp(cBC, cNBC, saturate((25000.0f / Input.vViewPosition.z)));
	
	
	return color;
}