#include <AdaptiveTesselation.h>

#define TRI_SAMPLE_LEVEL 0
#include <Triplanar.h>

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer OceanSettings : register( b1 )
{
	float3 OS_CameraPosition;
	float OS_SpecularPower;
	
	// Water-reflected sky color
	float3			OS_SkyColor;
	float			unused0;
	// The color of bottomless water body
	float3			OS_WaterbodyColor;

	// The strength, direction and color of sun streak
	float			OS_Shineness;
	float3			OS_SunDir;
	float			unused1;
	float3			OS_SunColor;
	float			unused2;
	
	// The parameter is used for fixing an artifact
	float3			OS_BendParam;

	// Perlin noise for distant wave crest
	float			OS_PerlinSize;
	float3			OS_PerlinAmplitude;
	float			unused3;
	float3			OS_PerlinOctave;
	float			unused4;
	float3			OS_PerlinGradient;

	// Constants for calculating texcoord from position
	float			OS_TexelLength_x2;
	float			OS_UVScale;
	float			OS_UVOffset;
}

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
};

struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside: SV_InsideTessFactor;
};

Texture2D TX_Texture0 : register( t0 );  
SamplerState SS_Linear : register( s0 );

static const float mTessellationFactor = 170.0f;


cbuffer DefaultHullShaderConstantBuffer : register( b1 )
{
	float H_EdgesPerScreenHeight;
	float H_Proj11;
	float H_GlobalTessFactor;
	float H_FarPlane;
	float2 H_ScreenResolution;
	float2 h_pad2;
};


float GetPostProjectionSphereExtent(float3 Origin, float Diameter)
{
    float4 ClipPos = mul(float4( Origin, 1.0 ), M_ViewProj);
    return abs(Diameter * M_Proj[1][1] / ClipPos.w);
}

float CalculateTessellationFactor(float3 Control0, float3 Control1)
{
    float e0 = distance(Control0,Control1);
    float3 m0 = (Control0 + Control1)/2;
    return max(1,H_GlobalTessFactor * GetPostProjectionSphereExtent(m0,e0));
}

//--------------------------------------------------------------------------------------
// Patch Constant Function
//--------------------------------------------------------------------------------------
ConstantOutputType TessPatchConstantFunction(InputPatch<VS_OUTPUT, 3> inputPatch, uint patchId : SV_PrimitiveID)
{   
    ConstantOutputType output;
  
	//float factor = max(1, (1.0f-(inputPatch[0].vViewPosition.z / 4000.0f)) * mTessellationFactor);
	//factor = min(factor, 100);
	float factor = 40.0f;
  
	float3 viewPosition[3];
	viewPosition[0] = mul(float4(inputPatch[0].vWorldPosition, 1), M_View);
	viewPosition[1] = mul(float4(inputPatch[1].vWorldPosition, 1), M_View);
	viewPosition[2] = mul(float4(inputPatch[2].vWorldPosition, 1), M_View);
	
	float far = 12000.0f;
	float tessPow = 1 / 6.0f;
	float dist[3];
	dist[0] = pow(saturate(length(viewPosition[0]) / far), tessPow);
	dist[1] = pow(saturate(length(viewPosition[1]) / far), tessPow);
	dist[2] = pow(saturate(length(viewPosition[2]) / far), tessPow);
	
	dist[0] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[0]));
	dist[1] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[1]));
	dist[2] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[2]));
	
	
	
	// Assign tessellation levels
    output.edges[0] = 0.5f * (dist[1] + dist[2]);
    output.edges[1] = 0.5f * (dist[2] + dist[0]);
    output.edges[2] = 0.5f * (dist[0] + dist[1]);
    output.inside   = (output.edges[0] + output.edges[1] + output.edges[2]) / 3.0f;
   
	output.edges[0] = 1.0f;
    output.edges[1] = 1.0f;
    output.edges[2] = 1.0f;
    output.inside   = 1.0f;
	
    return output;
}
//--------------------------------------------------------------------------------------
// Hull Shader
//--------------------------------------------------------------------------------------
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TessPatchConstantFunction")]
//[maxtessfactor(64.0)]
VS_OUTPUT HSMain(InputPatch<VS_OUTPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    VS_OUTPUT output;

	output.vTexcoord	 	= patch[pointId].vTexcoord; 	
	output.vTexcoord2		= patch[pointId].vTexcoord2;		
	output.vDiffuse			= patch[pointId].vDiffuse;			
	output.vWorldPosition 	= patch[pointId].vWorldPosition; 	
	output.vNormalVS		= patch[pointId].vNormalVS;	
	output.vNormalWS		= patch[pointId].vNormalWS;		
	//output.vViewPosition	= patch[pointId].vViewPosition;	
	//output.vPosition		= patch[pointId].vPosition;		
	
    return output;
}




[domain("tri")]
PS_INPUT DSMain(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<VS_OUTPUT, 3> patch)
{
    float4 vertexPosition;
    float2 texCoord;
	float2 texCoord2;
    float3 normalVS;
	float3 normalWS;
	float4 diffuse;
    PS_INPUT output;
    float3 viewPosition;
	float3 worldPosition;
  
    diffuse = uvwCoord.x * patch[0].vDiffuse + uvwCoord.y * patch[1].vDiffuse + uvwCoord.z * patch[2].vDiffuse;
	//vertexPosition = uvwCoord.x * patch[0].vPosition + uvwCoord.y * patch[1].vPosition + uvwCoord.z * patch[2].vPosition;
	//viewPosition = uvwCoord.x * patch[0].vViewPosition + uvwCoord.y * patch[1].vViewPosition + uvwCoord.z * patch[2].vViewPosition;
	worldPosition = uvwCoord.x * patch[0].vWorldPosition + uvwCoord.y * patch[1].vWorldPosition + uvwCoord.z * patch[2].vWorldPosition;
    texCoord = uvwCoord.x * patch[0].vTexcoord + uvwCoord.y * patch[1].vTexcoord + uvwCoord.z * patch[2].vTexcoord;
	texCoord2 = uvwCoord.x * patch[0].vTexcoord2 + uvwCoord.y * patch[1].vTexcoord2 + uvwCoord.z * patch[2].vTexcoord2;
	normalVS = uvwCoord.x * patch[0].vNormalVS + uvwCoord.y * patch[1].vNormalVS + uvwCoord.z * patch[2].vNormalVS;
	normalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	
	float distance = length(OS_CameraPosition - worldPosition);
	float dispMod = saturate((distance - 5000) * 0.0001f);
	
	//float scale = 1/(2000.0f);
	texCoord = (worldPosition.xz / 2048) + OS_UVOffset;
	float3 displacementFFT = TX_Texture0.SampleLevel(SS_Linear, texCoord, 0).xzy;
	float3 displacementRandom = TX_Texture0.SampleLevel(SS_Linear, 0.4f * texCoord + (worldPosition.zx / 2048 * 0.666f), 0).xzy;

	float3 displacement = lerp(displacementFFT, displacementRandom, dispMod);

						
	float3 vHeight = 1.0f * displacement;
	worldPosition += vHeight;
	
   /* vertexPosition.xyz += normalVS * (vHeight);
	
    output.vPosition = vertexPosition;
	output.vTexcoord2 = texCoords;
	output.vTexcoord = texCoords;
	output.vDiffuse  = diffuse;
	
	output.vWorldPosition = worldPosition;
	output.vViewPosition = viewPosition;
	
	output.vNormalWS = normalWS;
	output.vNormalVS = normalVS;*/
		
	//Output.vPosition = float4(Input.vPosition, 1);
	output.vPosition = mul( float4(worldPosition,1), M_ViewProj);
	output.vTexcoord2 = texCoord2;
	output.vTexcoord = texCoord;
	output.vDiffuse  = float4(dispMod.rrr,1);
	output.vNormalWS = normalWS;
	output.vNormalVS = normalVS;
	//output.vViewPosition = mul(float4(worldPosition,1), M_View).xyz;
	output.vWorldPosition = worldPosition;
	
    return output;
}