//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <DS_Defines.h>

#include <AtmosphericScattering.h>

cbuffer DS_ScreenQuadConstantBuffer : register( b0 )
{
	matrix SQ_InvProj; // Optimize out!
	matrix SQ_InvView;
	float3 SQ_LightDirectionVS;
	float SQ_ShadowmapSize;
	
	float4 SQ_LightColor;
	matrix SQ_ShadowView;
	matrix SQ_ShadowProj;
	
	float SQ_ShadowStrength;
	float SQ_ShadowAOStrength;
	float SQ_WorldAOStrength;
	float SQ_Pad;
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
Texture2D	TX_Shadowmap : register( t3 );




//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexCoord 		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
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
    float4 vPositionVS = mul(vProjectedPos, SQ_InvProj); //invViewProj == invProjection here  
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

float2 TexOffset( int u, int v )
{
    return float2( u * 1.0f/SQ_ShadowmapSize, v * 1.0f/SQ_ShadowmapSize );
}

float IsInShadow(float3 wsPosition, Texture2D shadowmap, SamplerComparisonState samplerState)
{
	float4 vShadowSamplingPos = mul(float4(wsPosition, 1), mul(SQ_ShadowView, SQ_ShadowProj));
	
	float2 projectedTexCoords;
	vShadowSamplingPos.xyz /= vShadowSamplingPos.w;
    projectedTexCoords[0] = vShadowSamplingPos.x/2.0f +0.5f;
    projectedTexCoords[1] = vShadowSamplingPos.y/-2.0f +0.5f;
	
	return shadowmap.SampleCmpLevelZero(samplerState, projectedTexCoords.xy, vShadowSamplingPos.z);
}

float ComputeShadowValue(float2 uv, float3 wsPosition, Texture2D shadowmap, SamplerComparisonState samplerState, float distance, float vertLighting)
{
	// Reconstruct VS World ShadowViewPosition from depth
	float4 vShadowSamplingPos = mul(float4(wsPosition, 1), mul(SQ_ShadowView, SQ_ShadowProj));
	
	float2 projectedTexCoords;
	vShadowSamplingPos.xyz /= vShadowSamplingPos.w;
    projectedTexCoords[0] = vShadowSamplingPos.x/2.0f +0.5f;
    projectedTexCoords[1] = vShadowSamplingPos.y/-2.0f +0.5f;
	
	float shadow = 1.0f;
	if( !(projectedTexCoords.x > 1 || projectedTexCoords.y > 1 ||
		projectedTexCoords.x < 0 || projectedTexCoords.y < 0))
	{
		float bias = lerp(0.00005f, 0.0001f, distance / 80000);
		
#if SHD_FILTER_16TAP_PCF
		//return shadowmap.SampleCmpLevelZero( samplerState, projectedTexCoords.xy, vShadowSamplingPos.z - 0.00001f);
		//return shadowmap.Sample(SS_Linear, projectedTexCoords).r > vShadowSamplingPos.z ? 1 : 0;
		
		//float dist = shadowmap.Sample(SS_Linear, projectedTexCoords).r - vShadowSamplingPos.z;
		
		//return dist * 10.0f;
		
		//PCF sampling for shadow map
		float sum = 0;
		float x, y;
		
		float dx = ddx(projectedTexCoords.xy);
		float dy = ddy(projectedTexCoords.xy);
	 
		float minValue = 999999.0f;
		/*for (y = -1.5; y <= 1.5; y += 1.0)
		{
			for (x = -1.5; x <= 1.5; x += 1.0)
			{
				
				minValue = min(minValue, shadowmap.SampleGrad(SS_Linear, projectedTexCoords.xy + TexOffset(x,y), dx, dy)).r;
			}
		}*/
		
		float scale = 1.0f;//1 + (minValue - vShadowSamplingPos.z) * 500.0f;
		
	 
		//perform PCF filtering on a 4 x 4 texel neighborhood
		for (y = -1.5; y <= 1.5; y += 1.0)
		{
			for (x = -1.5; x <= 1.5; x += 1.0)
			{
				sum += shadowmap.SampleCmpLevelZero( samplerState, projectedTexCoords.xy + TexOffset(x,y) * scale, vShadowSamplingPos.z - bias);
			}
		}
	 
		float shadowFactor = sum / 16.0;
	
		shadow *= shadowFactor;
#else
		shadow = shadowmap.SampleCmpLevelZero( samplerState, projectedTexCoords.xy, vShadowSamplingPos.z - bias);
#endif
	
		
	}
	
	float border;
	border = pow(abs(projectedTexCoords.x), 16.0f);
	border += pow(abs(projectedTexCoords.y), 16.0f);
	border += pow(abs(1.0f-projectedTexCoords.x), 16.0f);
	border += pow(abs(1.0f-projectedTexCoords.y), 16.0f);
	shadow = lerp(shadow, vertLighting, saturate(border));
	
	return saturate(shadow);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
	// Get screen UV
	float2 uv = Input.vTexCoord; 
	
	// Look up the diffuse color
	float4 diffuse = TX_Diffuse.Sample(SS_Linear, uv);
	float vertLighting = diffuse.a;
	
	// Get the second GBuffer
	float4 gb2 = TX_Nrm_SI_SP.Sample(SS_Linear, uv);
	
	// Decode the view-space normal back
	float3 normal = DecodeNormal(gb2.xy);
	
	normal = normalize(normal);
	
	// If we dont have a normal, just return the diffuse color
	if(abs(gb2.x + gb2.y) < 0.01f)
		return float4(diffuse.rgb, 1);
	
	// Get specular parameters
	float specIntensity = gb2.z;
	float specPower = gb2.w;
	
	// Reconstruct VS World Position from depth
	float expDepth = TX_Depth.Sample(SS_Linear, uv).r;
	float3 vsPosition = VSPositionFromDepth(expDepth, uv);
	float3 wsPosition = mul(float4(vsPosition, 1), SQ_InvView).xyz;
	
#if SHD_ENABLE
	//return float4(mul(float4(wsPosition, 1), mul(SQ_ShadowView, SQ_ShadowProj)).xyz, 1);
	
	// Get shadowing
	float shadow = ComputeShadowValue(uv, wsPosition, TX_Shadowmap, SS_Comp, vsPosition.z, vertLighting);
	
#else
	float shadow = vertLighting;
#endif
	//shadow = 1.0f;

	// Sunrays
	/*float3 vsDir = normalize(vsPosition);
	const int numSamples = 100;
	float stepSize = 1000.0f / numSamples;
	float shaft = 0.0f;
	for(float r=0;r < 1000.0f;r+=stepSize)
	{
		float3 vsRayPos = vsDir * r;
		float3 wsRayPos = mul(float4(vsRayPos, 1), SQ_InvView).xyz;
		
		float s = IsInShadow(wsRayPos, TX_Shadowmap, SS_Comp);
		
		shaft += s / numSamples;
	}*/
	
	
	// Compute specular lighting
	float3 V = normalize(-vsPosition);
	float3 H = normalize(SQ_LightDirectionVS + V );
	float spec = CalcBlinnPhongLighting(normal, H);
	float specMod = pow(dot(float3(0.333f,0.333f,0.333f), diffuse.rgb), 2);
	
	
	//return float4(diffuse.rgb, 1);
	
	// Apply sunlight
	float sunStrength = dot(SQ_LightColor.rgb, float3(0.333f,0.333f,0.333f));
	
	float vertAO = lerp(pow(saturate(vertLighting * 2), 2), 1.0f, 0.5f);
	float sun = saturate(dot(normalize(SQ_LightDirectionVS), normal) * shadow) * 1.0f;

	float3 specBare = pow(spec, specPower) * specIntensity * SQ_LightColor.rgb * sun;
	float3 specColored = saturate(lerp(specBare, specBare * diffuse.rgb, specMod));
	
	float shadowAO = lerp(1.0f, vertLighting, SQ_ShadowAOStrength);
	float worldAO = lerp(1.0f, vertLighting, SQ_WorldAOStrength);
	
	float3 litPixel = lerp( diffuse * SQ_ShadowStrength * sunStrength * shadowAO, 
							diffuse * SQ_LightColor * SQ_LightColor.a * worldAO, sun) 
				  + specColored;
	
	// Run scattering
	litPixel = ApplyAtmosphericScatteringGround(wsPosition, litPixel.rgb);
	

	
	// Fix indoor stuff
	//litPixel = lerp(diffuse * vertLighting, litPixel, vertLighting < 0.9f ? 0 : 1);
	//diffuse.rgb = lerp(diffuse.rgb, 1.0f, clamp(shaft, 0.0f, 0.4f));
	
	
	//return float4(sun.rgb, 1);
	//return float4(vertLighting.rrr, 1);
	return float4(litPixel.rgb, 1);
	//return float4(pow(spec, specPower) * specIntensity.xxx * diffuse.rgb * SQ_LightColor.rgb,1);
	
}

