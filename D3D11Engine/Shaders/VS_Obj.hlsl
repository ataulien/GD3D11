//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer Matrices_PerInstances : register( b1 )
{
	matrix M_World;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTex1		: TEXCOORD0;
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
	
	//Output.vPosition = float4(Input.vPosition, 1);
	Output.vPosition = mul( float4(Input.vPosition,1), mul(M_World, M_ViewProj) );
	Output.vTexcoord2 = Input.vTex1;
	Output.vTexcoord = Input.vTex1;
	Output.vDiffuse  = 1.0f;
	Output.vNormalVS = mul(Input.vNormal, (float3x3)mul(M_World, M_View));
	//Output.vViewPosition = mul(float4(Input.vPosition,1), mul(M_World, M_View));
	Output.vWorldPosition = mul(float4(Input.vPosition,1), M_World).rgb;
	
	return Output;
}

