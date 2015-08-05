//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
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
	uint InstanceRemapIndex : INSTANCE_REMAP_INDEX;
};

struct InstanceData
{
	float4x4 InstanceWorldMatrix;
	uint InstanceColor;
	uint pad[3];
};

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	
	float4 vPosition		: SV_POSITION;
};

/** Structured buffer for the remapped instances */
StructuredBuffer<InstanceData> InstanceSB : register(t0);

float4 DWORDToFloat4(uint color)
{
	float a = (color >> 24) / 255.0f;
	float r = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >> 8 ) & 0xFF) / 255.0f;
	float b = (color & 0xFF) / 255.0f;

	return float4(r,g,b,a);
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	// Get instancedata from our buffer
	InstanceData inst = InstanceSB[Input.InstanceRemapIndex];
	
	float3 wpos = mul(float4(Input.vPosition,1), inst.InstanceWorldMatrix).xyz;
	Output.vPosition = mul( float4(wpos,1), M_ViewProj);
	
	Output.vTexcoord2 = Input.vTex2;
	Output.vTexcoord = Input.vTex1;
	Output.vDiffuse  = DWORDToFloat4(inst.InstanceColor);
	Output.vNormalVS = mul(Input.vNormal, mul((float3x3)inst.InstanceWorldMatrix, (float3x3)M_View));
	Output.vViewPosition = mul(float4(wpos,1), M_View);
	//Output.vWorldPosition = positionWorld;
	
	return Output;
}

