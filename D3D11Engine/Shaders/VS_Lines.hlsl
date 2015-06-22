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
	float4 vPosition	: POSITION;
	float4 vDiffuse		: DIFFUSE;
};

struct VS_OUTPUT
{
	float4 vDiffuse			: TEXCOORD0;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( float4(Input.vPosition.xyz,1), mul(M_World, M_ViewProj) );
	Output.vDiffuse  = Input.vDiffuse;
		
	Output.vPosition.z *= Input.vPosition.w;
		
	return Output;
}

