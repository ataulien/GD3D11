//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices : register( b0 )
{
	matrix M_WorldView;
	matrix M_Proj;	
};


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
};

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( float4(Input.vPosition,1), mul(M_WorldView, M_Proj) );
	Output.vTexcoord = Input.vTex1;
	
	return Output;
}
