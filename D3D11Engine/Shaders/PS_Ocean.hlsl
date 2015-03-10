//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <FFFog.h>
#include <DS_Defines.h>

cbuffer OceanSettings : register( b2 )
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

cbuffer OceanSettings : register( b3 )
{
	float3 OPP_LocalEye;
	float OPP_Pad1;
	
	float3 OPP_PatchPosition;
	float OPP_Pad2;
}

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_Clamp : register( s1 );
SamplerState SS_Cube : register( s2 );

Texture2D	TX_Normal : register( t0 );
Texture1D	TX_Fresnel : register( t1 );
TextureCube	TX_ReflectionCube : register( t2 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
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


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{
	float texel_length_x2 = OS_TexelLength_x2;
	float3 skyColor = OS_SkyColor;
	float3 waterbodyColor = OS_WaterbodyColor;
	float3 bendParam = OS_BendParam;

	float3 localPos = (Input.vWorldPosition - OPP_PatchPosition) / 2048;
	
	float3 viewDir = normalize(OPP_LocalEye - localPos);
	viewDir = viewDir.xzy * float3(1,1,1); // Nvidias sim has z-up
	
	float3 normal = TX_Normal.Sample(SS_Linear, Input.vTexcoord).xyz;
	normal = normalize(float3(normal.xy, texel_length_x2));
	//normal = float3(0,0,1);
	
	// Reflected ray
	float3 reflect_vec = reflect(-viewDir, normal);
	
	// dot(N, V)
	float cos_angle = (dot(normal, viewDir));
	float4 ramp = TX_Fresnel.Sample(SS_Clamp, cos_angle);
	//float4 ramp = TX_Fresnel.Sample(SS_Linear, Input.vTexcoord);
	
	// A workaround to deal with "indirect reflection vectors" (which are rays requiring multiple
	// reflections to reach the sky).
	if (reflect_vec.z < bendParam.x)
		ramp = lerp(ramp, bendParam.z, (bendParam.x - reflect_vec.z)/(bendParam.x - bendParam.y));
	reflect_vec.z = max(0, reflect_vec.z);
	//reflect_vec = reflect_vec.xzy * float3(1,1,1);
	
	// sample reflection cube
	float3 reflection = TX_ReflectionCube.Sample(SS_Cube, reflect_vec).xyz;
	// Hack bit: making higher contrast
	reflection = reflection * reflection * 2.5f;
	
	// Blend with predefined sky color
	float3 reflected_color = lerp(skyColor, reflection, ramp.y);

	// Combine waterbody color and reflected color
	float3 water_color = lerp(waterbodyColor, reflected_color, ramp.x);
	
	// --------------- Sun spots

	float3 sunOrange = float3(0.6,0.3,0.1) * 2.0f;
	float3 waterLighting = lerp(sunOrange * 0.02f, 1.0f, clamp(1-pow(1-OS_SunDir.y, 16), 0.25, 1.0f));
	float3 sunColor = lerp(sunOrange, 1.0f, OS_SunDir.y) * 5.0f;
	
	float cos_spec = clamp(dot(reflect_vec, OS_SunDir.xzy * float3(1,1,1)), 0, 1);
	float sun_spot = pow(cos_spec, OS_Shineness);
	water_color *= waterLighting;
	water_color += sunColor * sun_spot;
	
	
	
	float4 color = 1.0f;
	color.rgb = water_color;
	//color.rgb = 0.5f;
	
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, 1.0f);
		
	output.vNrm_SI_SP.xy = 0.0f;//EncodeNormal(normalize(Input.vNormalVS));
	output.vNrm_SI_SP.z = 1.0f;
	output.vNrm_SI_SP.w = OS_SpecularPower;
	return output;
}
