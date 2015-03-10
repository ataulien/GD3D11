//--------------------------------------------------------------------------------------
// World/VOB-Pixelshader for G2D3D11 by Degenerated
//--------------------------------------------------------------------------------------
#include <AtmosphericScattering.h>
#include <FFFog.h>
#include <DS_Defines.h>

// Uncomment to enable LOD
#define USE_MIP_LOD

// Uncomment to enable silhouette clipping
//#define USE_SILHOUETTE_CLIPPING

static const int		MAX_SAMPLES			= 20;
static const int		MIN_SAMPLES			= 4;
static const float 		TEX_SCALE 			= 1.0f;
static const float		SPECULAR_POWER		= 128.0f;
static const float		SPECULAR_MULT		= 8.0f;
static const float		LIGHT_RANGE			= 128.0f;
static const int 		TEXTURE_DIMS		= 512;
static const int 		LOD_THRESHOLD		= 4;
static const float HEIGHT_MAP_SCALE = 0.1f;

cbuffer MI_MaterialInfo : register( b2 )
{
	float MI_SpecularIntensity;
	float MI_SpecularPower;
	float MI_NormalmapStrength;
	float MI_ParallaxOcclusionStrength;
}

cbuffer POS_MaterialInfo : register( b3 )
{
	float3 OS_AmbientColor;
	float OS_Pad;
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

float2 calculateParallaxCorrectedTexCoord(PS_INPUT input, float2 parallaxOffsetTS, float3 viewTS)
{
	float2 texSampleBase = input.uv_coords;
	
	float  fMipLevel;      
    float  fMipLevelInt;    // mip level integer portion
    float  fMipLevelFrac;   // mip level fractional amount for blending in between levels

    float  fMinTexCoordDelta;
    float2 dTexCoords;

	
#ifdef USE_MIP_LOD
	// Compute the current gradients:
	float2 fTexCoordsPerSize = input.vTexcoord * TEXTURE_DIMS;

	// Compute all 4 derivatives in x and y in a single instruction to optimize:
	float2 dxSize, dySize;
	float2 dx, dy;

	float4( dxSize, dx ) = ddx( float4( fTexCoordsPerSize, input.vTexcoord ) );
	float4( dySize, dy ) = ddy( float4( fTexCoordsPerSize, input.vTexcoord ) );
	
    // Find min of change in u and v across quad: compute du and dv magnitude across quad
    dTexCoords = dxSize * dxSize + dySize * dySize;

    // Standard mipmapping uses max here
    fMinTexCoordDelta = max( dTexCoords.x, dTexCoords.y ) * 0.5f;

    // Compute the current mip level  (* 0.5 is effectively computing a square root before )
    fMipLevel = max( 0.5 * log2( fMinTexCoordDelta ), 0 );
       
   // Multiplier for visualizing the level of detail (see notes for 'nLODThreshold' variable
   // for how that is done visually)
   float4 cLODColoring = float4( 1, 1, 3, 1 );

   float fOcclusionShadow = 1.0;

   if ( fMipLevel <= (float) LOD_THRESHOLD )
   {
 #endif
	// Calculate directions
	float3 N = normalize( input.vNormalVS );
	float3 E = normalize( viewTS );



	//===============================================//
	// Parallax occlusion mapping offset computation //
	//===============================================//

	// Utilize dynamic flow control to change the number of samples per ray 
	// depending on the viewing angle for the surface. Oblique angles require 
	// smaller step sizes to achieve more accurate precision for computing displacement.
	// We express the sampling rate as a linear function of the angle between 
	// the geometric normal and the view direction ray:
	int nNumSteps = (int)lerp( MAX_SAMPLES, MIN_SAMPLES, dot( E, N ) );

	// Intersect the view ray with the height field profile along the direction of
	// the parallax offset ray (computed in the vertex shader. Note that the code is
	// designed specifically to take advantage of the dynamic flow control constructs
	// in HLSL and is very sensitive to specific syntax. When converting to other examples,
	// if still want to use dynamic flow control in the resulting assembly shader,
	// care must be applied.
	// 
	// In the below steps we approximate the height field profile as piecewise linear
	// curve. We find the pair of endpoints between which the intersection between the 
	// height field profile and the view ray is found and then compute line segment
	// intersection for the view ray and the line segment formed by the two endpoints.
	// This intersection is the displacement offset from the original texture coordinate.
	// See the above paper for more details about the process and derivation.
	//

	

	float fCurrHeight = 0.0;
	float fStepSize   = 1.0 / (float) nNumSteps;
	float fPrevHeight = 1.0;
	float fNextHeight = 0.0;

	int    nStepIndex = 0;
	bool   bCondition = true;

	float2 vTexOffsetPerStep = fStepSize * parallaxOffsetTS;
	float2 vTexCurrentOffset = input.vTexcoord;
	float  fCurrentBound     = 1.0;
	float  fParallaxAmount   = 0.0;

	float2 pt1 = 0;
	float2 pt2 = 0;

	float2 texOffset2 = 0;

	while ( nStepIndex < nNumSteps ) 
	{
		vTexCurrentOffset -= vTexOffsetPerStep;

		// Sample height map
		fCurrHeight = hsgTex.SampleGrad( SS_Linear, vTexCurrentOffset, dx, dy ).r;
		
		fCurrentBound -= fStepSize;

		if ( fCurrHeight > fCurrentBound ) 
		{   
			pt1 = float2( fCurrentBound, fCurrHeight );
			pt2 = float2( fCurrentBound + fStepSize, fPrevHeight );

			texOffset2 = vTexCurrentOffset - vTexOffsetPerStep;

			nStepIndex = nNumSteps + 1;
			fPrevHeight = fCurrHeight;
		}
		else
		{
			nStepIndex++;
			fPrevHeight = fCurrHeight;
		}
	}   

	float fDelta2 = pt2.x - pt2.y;
	float fDelta1 = pt1.x - pt1.y;

	float fDenominator = fDelta2 - fDelta1;

	// SM 3.0 requires a check for divide by zero, since that operation will generate
	// an 'Inf' number instead of 0, as previous models (conveniently) did:
	if ( fDenominator == 0.0f )
	{
		fParallaxAmount = 0.0f;
	}
	else
	{
		fParallaxAmount = (pt1.x * fDelta2 - pt2.x * fDelta1 ) / fDenominator;
	}

	
	
	float2 vParallaxOffset = parallaxOffsetTS * (1 - fParallaxAmount );

	// The computed texture offset for the displaced point on the pseudo-extruded surface:
	texSampleBase = input.vTexcoord - vParallaxOffset;
	
#ifdef USE_MIP_LOD
	 }
	 
	 if ( fMipLevel > (float)(LOD_THRESHOLD - 1) )
      {
         // Lerp based on the fractional part:
         fMipLevelFrac = modf( fMipLevel, fMipLevelInt);

         // Lerp the texture coordinate from parallax occlusion mapped coordinate to bump mapping
         // smoothly based on the current mip level:
         texSampleBase = lerp( texSampleBase, input.uv_coords, fMipLevelFrac );
     }
#endif
	return texSampleBase;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
DEFERRED_PS_OUTPUT PSMain( PS_INPUT Input ) : SV_TARGET
{
	float4 color = TX_Texture0.Sample(SS_Linear, Input.vTexcoord);

	// WorldMesh can always do the alphatest
	DoAlphaTest(color.a);
	
	float3 nrmmap = TX_Texture1.Sample(SS_Linear, Input.vTexcoord).xyz * 2 - 1;
	nrmmap.xy *= -1.0f;
	nrmmap.xy *= MI_NormalmapStrength;
	nrmmap = normalize(nrmmap);
	
    float3x3 TBN = transpose(cotangent_frame( Input.vNormalVS, -Input.vViewPosition, Input.vTexcoord ));

	float3 VTS = mul(TBN, Input.vViewPosition);
	float3 NTS = mul(TBN, Input.vNormalVS);
	
	// Compute initial parallax displacement direction:
    float2 vParallaxDirection = normalize( VTS.xy );
	
	// The length of this vector determines the furthest amount of displacement:
    float fLength         = length( VTS );
    float fParallaxLength = sqrt( fLength * fLength - VTS.z * VTS.z ) / VTS.z; 
          
	// Compute the actual reverse parallax displacement vector:
    float2 vParallaxOffsetTS = vParallaxDirection * fParallaxLength;
		  
	// Need to scale the amount of displacement to account for different height ranges
    // in height maps. This is controlled by an artist-editable parameter:
    parallaxOffsetTS *= HEIGHT_MAP_SCALE;
		  
	//color.rgb = ApplyAtmosphericScatteringGround(Input.vWorldPosition, color.rgb);
	
	DEFERRED_PS_OUTPUT output;
	output.vDiffuse = float4(color.rgb, Input.vDiffuse.y * OS_AmbientColor.y);
		
	output.vNrm_SI_SP.xy = EncodeNormal(nrm);
	output.vNrm_SI_SP.z = MI_SpecularIntensity;
	output.vNrm_SI_SP.w = MI_SpecularPower;
	return output;
}

