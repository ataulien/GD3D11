#include <FFConstantBuffer.h>

/** Computes the fog-term */
float3 ComputeFog(float3 viewPosition, float3 color)
{
	float distance = length(viewPosition);
	
	float l = saturate((distance-FF_FogNear)/(FF_FogFar-FF_FogNear));
	//l = lerp(0, l, FF_FogWeight);
	
	return lerp(color.rgb, FF_FogColor, l);
}