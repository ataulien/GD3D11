//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer ParticlePointShadingConstantBuffer : register( b1 )
{
	matrix AR_RainView;
	matrix AR_RainProj;
};


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float4 vDiffuse		: DIFFUSE;
    float2 vSize        : SIZE;
    unsigned int type   : TYPE;
    float3 vVelocity    : VELOCITY;
};

struct VS_OUTPUT
{
	float3 vPosition		: POSITION;
	float4 vDiffuse			: DIFFUSE;
    float2 vSize            : SIZE;
    int    type             : TYPE;
    float3 vVelocity        : VELOCITY;
};

SamplerComparisonState SS_Comp : register( s2 );
Texture2D	TX_RainShadowmap : register( t0 );

float IsWet(float3 wsPosition, Texture2D shadowmap, SamplerComparisonState samplerState, matrix viewProj)
{
	float4 vShadowSamplingPos = mul(float4(wsPosition, 1), viewProj);
	
	float2 projectedTexCoords;
	vShadowSamplingPos.xyz /= vShadowSamplingPos.w;
    projectedTexCoords[0] = vShadowSamplingPos.x/2.0f +0.5f;
    projectedTexCoords[1] = vShadowSamplingPos.y/-2.0f +0.5f;
	
	float bias = -0.001f;
	return shadowmap.SampleCmpLevelZero( samplerState, projectedTexCoords.xy, vShadowSamplingPos.z - bias);
}



//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	float wet = IsWet(Input.vPosition.xyz, TX_RainShadowmap, SS_Comp, mul(AR_RainView, AR_RainProj));
	
	// Scale intensity
	Input.vDiffuse.a *= wet;
	
	Output.vPosition = Input.vPosition;
	Output.vDiffuse  = Input.vDiffuse; //float4(Input.vDiffuse.gba, pow(Input.vDiffuse.r, 2.2f));
    Output.vSize = Input.vSize;
    Output.vVelocity = Input.vVelocity;
    Output.type = Input.type;
	return Output;
}
