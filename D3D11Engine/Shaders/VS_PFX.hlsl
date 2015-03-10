//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer PFXVS_ConstantBuffer : register(b0)
{
	matrix PFXVS_InvProj;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float2 vTex1		: TEXCOORD0;
	uint vertexID		: SV_VertexID;
};

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//------------------------------------------------------------------ --------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
			
	Output.vPosition = float4(Input.vPosition, 1);
	Output.vTexcoord = Input.vTex1;
	
	float3 positionVS = mul(float4(Input.vPosition, 1), PFXVS_InvProj).xyz;
	Output.vEyeRay = float3(positionVS.xy / positionVS.z, 1.0f);
	
	return Output;
}

