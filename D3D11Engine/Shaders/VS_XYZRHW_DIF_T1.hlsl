//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Viewport : register( b0 )
{
	float2 V_ViewportPos;
	float2 V_ViewportSize;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float4 vDiffuse		: DIFFUSE;
	float4 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
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
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	float3 p = Input.vPosition.xyz;

	p.xy -= V_ViewportPos;

	p.xy = p.xy * 2.0f - V_ViewportSize;
	p.xy /= V_ViewportSize; 
	
	p.y = -p.y;
	
	//p.xy /= float2(1920,1080); 
			
	Output.vPosition = float4(p, 1);
	
	//Output.vPosition = mul( float4(Input.vPosition,1), M_WorldViewProj );
	Output.vTexcoord2 = Input.vTexcoord;
	Output.vTexcoord = Input.vTexcoord;
	Output.vDiffuse  = Input.vDiffuse;
	Output.vNormalVS = float3(0,0,0);//mul(Input.vNormal, (float3x3)M_WorldView);
	Output.vViewPosition = float3(0,0,0);
	Output.vWorldPosition = Input.vPosition.xyz;
	
	return Output;
}

