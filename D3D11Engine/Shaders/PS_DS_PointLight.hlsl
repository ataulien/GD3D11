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
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Diffuse : register( t0 );
Texture2D	TX_Nrm_SI_SP : register( t1 );
Texture2D	TX_Depth : register( t2 );

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

float GetShadow(float2 uv)
{
	//return 1;
	// Get light direction
	float2 lightDir = PL_LightScreenPos.xy - uv;
	float distance = length(lightDir);
	lightDir /= distance; // Normalize the direction
	
	// Calculate ray steps size
	const int numSteps = 100;
	float stepSize = distance / numSteps;
	
	//float depthLight = TX_Depth.Sample(SS_Linear, PL_LightScreenPos).r;
	float depthTarget = TX_Depth.Sample(SS_Linear, uv).r;
	
	//return depthTarget;
	float dx = ddx(uv.xy);
	float dy = ddy(uv.xy);
	
	float2 ray = PL_LightScreenPos.xy;
	for(int i=0;i<numSteps;i++)
	{
		ray += lightDir * stepSize;
		
		float depthRay = TX_Depth.SampleGrad(SS_Linear,ray, dx, dy);
		
		if(depthRay < PL_LightScreenPos.z)
			return 0;
	}
	
	return 1;
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

