//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------

#include <include/SamplerStates.h>
#include <include/InputDeclerations.h>
#include <include/FixedFunctionPipeline.h>
 
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSMain( PS_INPUT_FAT Input ) : SV_TARGET
{
	float4 color = GetColorFromStates(Input.vDiffuse.bgra, Input.vTexcoord, Input.vTexcoord2, g_samLinear);
	
	clip(color.a - alphaRef * alphaRefEnabled);
	
	if(waterLevel > -99999.0f && Input.vObjectPosition.y < waterLevel)
		discard;
	
	float3 lighting = lerp(0, ambientLight, lightEnabled) + lerp(1, ComputeDynamicLighting(Input.vNormalVS, Input.vWorldPosition).rgb, lightEnabled); // Apply lighting
	color.rgb *= lighting;
	
	color.rgb = ComputeFog(Input.vWorldPosition, color.rgb);
	
	return color;
}

