//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

static const int NUM_MAX_BONES = 96;

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer Matrices_PerInstances : register( b1 )
{
	matrix M_World;
	float PI_ModelFatness;
	float3 PI_Pad1;
};

cbuffer BoneTransforms : register( b2 )
{
	matrix BT_Transforms[NUM_MAX_BONES];
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition[4]	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTex1		: TEXCOORD0;
	float4 vDiffuse		: DIFFUSE;
	uint4 BoneIndices : BONEIDS;
	float4 Weights 	: WEIGHTS;
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

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	float3 position = float3(0,0,0);
	for(int i=0;i<4;i++)
	{
		position += Input.Weights[i] * mul(float4(Input.vPosition[i], 1), BT_Transforms[Input.BoneIndices[i]]).xyz;
	}
	
	
	float3 positionWorld = mul(float4(position + PI_ModelFatness * Input.vNormal,1), M_World).xyz;
	
	//Output.vPosition = float4(Input.vPosition, 1);
	Output.vPosition = mul( float4(positionWorld,1), M_ViewProj);
	Output.vTexcoord2 = Input.vTex1;
	Output.vTexcoord = Input.vTex1;
	Output.vDiffuse  = Input.vDiffuse;
	Output.vNormalVS = mul(Input.vNormal, (float3x3)mul(M_World, M_View));
	Output.vViewPosition = mul(float4(positionWorld,1),M_View).xyz;
	//Output.vWorldPosition = positionWorld;
	
	return Output;
}

