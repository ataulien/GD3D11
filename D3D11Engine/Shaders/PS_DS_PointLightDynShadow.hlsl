//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

cbuffer DS_PointLightConstantBuffer : register( b0 )
{
	float4 PL_Color;
	
	float PL_Range;
	float3 Pl_PositionWorld;
	
	float PL_Outdoor;
	float3 Pl_PositionView;
	
	float2 PL_ViewportSize;
	float2 PL_Pad2;
	
	matrix PL_InvProj; // Optimize out!
	matrix PL_InvView; // Optimize out!
	
	float3 PL_LightScreenPos;
	float PL_Pad3;
	
	matrix PL_ShadowView; // Optimize out!
	matrix PL_ShadowProj; // Optimize out!
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
SamplerComparisonState SS_Comp : register( s2 );
Texture2D	TX_Diffuse : register( t0 );
Texture2D	TX_Nrm_SI_SP : register( t1 );
Texture2D	TX_Depth : register( t2 );
TextureCube	TX_ShadowCube : register( t3 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition		: SV_POSITION;
};

float3 VSPositionFromDepth(float depth, float2 vTexCoord)
{
    // Get the depth value for this pixel
    float z = depth; 
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2 - 1;
    float y = (1 - vTexCoord.y) * 2 - 1;
    float4 vProjectedPos = float4(x, y, z, 1.0f);
    // Transform by the inverse projection matrix
    float4 vPositionVS = mul(vProjectedPos, PL_InvProj); //invViewProj == invProjection here  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;   
}

//--------------------------------------------------------------------------------------
// Blinn-Phong Lighting Reflection Model
//--------------------------------------------------------------------------------------
float CalcBlinnPhongLighting(float3 N, float3 H )
{
    return saturate(dot(N,H));
}

float IsInShadow(float3 wsPosition, TextureCube shadowCube, SamplerComparisonState samplerState, float bias = 0.01f)
{
	float4 vShadowSamplingPos = mul(float4(wsPosition, 1), mul(PL_ShadowView, PL_ShadowProj));
	
	float2 projectedTexCoords;
	vShadowSamplingPos.xyz /= vShadowSamplingPos.w;
    projectedTexCoords[0] = vShadowSamplingPos.x/2.0f +0.5f;
    projectedTexCoords[1] = vShadowSamplingPos.y/-2.0f +0.5f;
	
	// Get dir from pointlight to pixel
	float distance = length(wsPosition - Pl_PositionWorld);
	float3 dir = wsPosition - Pl_PositionWorld;
	
	float zFar = PL_Range * 2.0f;
	float zNear = 50.0f;

	float s = shadowCube.Sample(SS_Linear, dir).r;
	s *= zFar;
	//s = -51.2820549f / (s - 1.02564108f);
	//s = (zNear * zFar) / (zFar - s * (zFar - zNear));
	
	//distance = ((zFar * distance) - (zNear * zFar)) / ((zFar - zNear) * distance);
	
	distance = distance / zFar;
	
	//return s < distance - bias?  0.0f : 1.0f;
	return shadowCube.SampleCmpLevelZero(samplerState, dir, distance - bias);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	// Get screen UV
	float2 uv = Input.vPosition.xy / PL_ViewportSize; 
	
	// Look up the diffuse color
	float4 diffuse = TX_Diffuse.Sample(SS_Linear, uv);
	
	// Get the second GBuffer
	float4 gb2 = TX_Nrm_SI_SP.Sample(SS_Linear, uv);
	
	// Decode the view-space normal back
	float3 normal = normalize(DecodeNormal(gb2.xy));
	
	// Get specular parameters
	float specIntensity = gb2.z;
	float specPower = gb2.w;
	
	// Reconstruct VS World Position from depth
	float expDepth = TX_Depth.Sample(SS_Linear, uv).r;
	float3 vsPosition = VSPositionFromDepth(expDepth, uv);
	float3 wsPosition = mul(float4(vsPosition,1), PL_InvView);
	
	//return float4(normalize(wsPosition - Pl_PositionWorld), 1.0f);
	
	// Compute flat normal
	//float3 flatNormal = normalize(cross(ddx(vsPosition),ddy(vsPosition)));
	
	//if(Input.vPosition.z > expDepth)
	//	discard;
	
	//return float4(Pl_PositionView, 1);
	
	// Get direction and distance from the light to that position 
	float3 lightDir = Pl_PositionView - vsPosition;
	float distance = length(lightDir);
	lightDir /= distance; // Normalize the direction
	
	// Do some simple NdL-Lighting
	float ndl = max(0, dot(lightDir, normal));
	
	// Apply dynamic shadow
	ndl *= IsInShadow(wsPosition, TX_ShadowCube, SS_Comp);
	//return float4(ndl.rrr,1);
	
	// Get rid of lighting on the backfaces of normalmapped surfaces
	//ndl *= saturate(dot(lightDir, flatNormal)  / 0.00001f);
	
	// Compute range falloff
	float falloff = pow(saturate(1.0f - (distance / PL_Range)), 1.2f); 
	//float falloff = saturate(1.0f / (pow(distance / PL_Range * 2, 2)));
	
	// Compute specular lighting
	float3 V = normalize(-Pl_PositionView);
	float3 H = normalize(lightDir + V );
	float spec = CalcBlinnPhongLighting(normal, H);
	float specMod = pow(dot(float3(0.333f,0.333f,0.333f), diffuse.rgb), 2);
	float3 specBare = pow(spec, specPower) * specIntensity * PL_Color.rgb * falloff;
	float3 specColored = lerp(specBare, specBare * diffuse.rgb, specMod);
	
	float3 color = falloff * ndl * PL_Color.rgb;
	color = saturate(color);
	
	// Blend this with the lights color and the worlds diffuse color
	// Also apply specular lighting
	float3 lighting = color * diffuse.rgb + specColored;
	
	//lighting = GetShadow(uv);
	
	// If this is an indoor-light, and this pixel already gets light from the sun, don't light it here.
	//float indoor = 1.0f - PL_Outdoor;
	//float indoorPixel = diffuse.a < 0.5f ? 1.0f : 0.0f;
	//lighting *= saturate(PL_Outdoor + indoor * indoorPixel);
	//lighting = indoorPixel;
	
	//return float4(0.2f,0.2f,0.2f,1);
	//return float4(ndl.rrr,1);
	return float4(saturate(lighting),1);
}

