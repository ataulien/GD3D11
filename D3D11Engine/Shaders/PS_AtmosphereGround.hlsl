//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <AtmosphericScattering.h>
#include <FFFog.h>
#include <DS_Defines.h>
#include <Toolbox.h>

cbuffer MI_MaterialInfo : register( b2 )
{
	float MI_SpecularIntensity;
	float MI_SpecularPower;
	float MI_NormalmapStrength;
	float MI_ParallaxOcclusionStrength;
}


/*cbuffer POS_MaterialInfo : register( b3 )
{
	float3 OS_AmbientColor;
	float OS_Pad;
}*/

cbuffer DIST_Distance : register( b3 )
{
	float DIST_DrawDistance;
	float3 DIST_Pad;
}

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
SamplerState SS_Linear : register( s0 );
SamplerState SS_samMirror : register( s1 );
Texture2D	TX_Texture0 : register( t0 );
Texture2D	TX_Texture1 : register( t1 );


//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalVS		: TEXCOORD4;
	float3 vViewPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};



float4 retF(float f)
{
	return float4(f,f,f,1);
}

float3x3 cotangent_frame( float3 N, float3 p, float2 uv )
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx( p );
    float3 dp2 = ddy( p );
    float2 duv1 = ddx( uv );
    float2 duv2 = ddy( uv );
 
    // solve the linear system
    float3 dp2perp = cross( dp2, N );
    float3 dp1perp = cross( N, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
	// Negate because of left-handedness
	//T *= -1;
	//B *= -1;
 
    // construct a scale-invariant frame 
    float invmax = rsqrt( max( dot(T,T), dot(B,B) ) );
    return float3x3( T * invmax, B * invmax, N );
}

/** Magic TBN-Calculation function */
float3 perturb_normal( float3 N, float3 V, float2 texcoord, SamplerState samplerState, float normalmapDepth = 1.0f)
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    float3 nrmmap = TX_Texture1.Sample(samplerState, texcoord).xyz * 2 - 1;
	nrmmap.xy *= -1.0f;
	nrmmap.xy *= normalmapDepth;
	nrmmap = normalize(nrmmap);
	
    float3x3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( mul(transpose(TBN), nrmmap) );
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);
	
	//ClipDistanceEffect(length(Input.vViewPosition), DIST_DrawDistance, color.r * 2 - 1, 500.0f);
	
	//float3 nrm = perturb_normal(Input.vNormalVS, Input.vViewPosition, Input.vTexcoord, SS_Linear, MI_NormalmapStrength);
	float3 nrm = 1.0f;
	
	//float specIntens = TX_Texture1.Sample(SS_Linear, Input.vTexcoord).a;
	
	// Apply static lighting
	//color *= ;
	
	// WorldMesh can always do the alphatest
	//DoAlphaTest(color.a);

	//color.rgb = ApplyAtmosphericScatteringGround(Input.vWorldPosition, color.rgb);
	
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, Input.vDiffuse.y);
	
	output.vNrm_SI_SP.xy = EncodeNormal(nrm);
	output.vNrm_SI_SP.z = MI_SpecularIntensity;
	output.vNrm_SI_SP.w = MI_SpecularPower;
	return output;
}

