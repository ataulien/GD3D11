/** Input declerations */

/** Fat Pixel-Shader input */
struct PS_INPUT_FAT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vWorldPosition 	: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vObjectPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};