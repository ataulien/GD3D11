//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

SamplerState SS_Linear : register( s0 );
Texture2D TX_Texture0 : register( t0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTex1		: TEXCOORD0;
	float2 vTex2		: TEXCOORD1;
	float4 vDiffuse		: DIFFUSE;
	float4x4 InstanceWorldMatrix : INSTANCE_WORLD_MATRIX;
	float4 InstanceColor	     : INSTANCE_COLOR;
};

struct VS_OUTPUT
{
	float3 vViewPosition	: TEXCOORD0;
	float3 vNormalWS		: TEXCOORD1; 
	float3 vNormalVS		: TEXCOORD2; 
	float2 vTexcoord		: TEXCOORD3;
	float2 vTexcoord2		: TEXCOORD4;
	float4 vDiffuse			: TEXCOORD5;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
		
	Output.vTexcoord = Input.vTex1;
	Output.vTexcoord2 = Input.vTex2;
	Output.vNormalVS = normalize(mul(Input.vNormal, (float3x3)mul(Input.InstanceWorldMatrix, M_View)));
	Output.vNormalWS = normalize(mul(Input.vNormal, (float3x3)Input.InstanceWorldMatrix));
	Output.vViewPosition = mul(float4(Input.vPosition,1), mul(Input.InstanceWorldMatrix, M_View)).xyz;
	Output.vDiffuse = Input.InstanceColor;
	
	return Output;
}

