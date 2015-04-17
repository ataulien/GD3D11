
void ClipDistanceEffect(float viewSpaceDepth, float drawDistance, float noise, float noiseScale)
{
	if(viewSpaceDepth + noise * noiseScale < drawDistance)
		discard;
}