#ifndef _HDR_H
#define _HDR_H

//static const float3 LUM_CONVERT = float3(0.299f, 0.587f, 0.114f); 
//static const float3 LUM_CONVERT  = float3(0.2125f, 0.7154f, 0.0721f);
static const float3 LUM_CONVERT  = float3(0.333f, 0.333f, 0.333f);

cbuffer HDR_Settings : register(b0)
{
	float HDR_MiddleGray;
	float HDR_LumWhite;
	float HDR_Threshold;
	float HDR_BloomStrength;
};

float3 ToneMap_Reinhard(float3 vColor, Texture2D lumTex, SamplerState samplerState)
{
	// Get the calculated average luminance
	float fLumAvg = lumTex.SampleLevel(samplerState, float2(0.5f, 0.5f), 9).r;

	// Calculate the luminance of the current pixel
	float fLumPixel = dot(vColor, LUM_CONVERT);
	
	vColor.rgb *= HDR_MiddleGray / (fLumAvg.r + 0.001f);
    vColor.rgb *= (1.0f + vColor/HDR_LumWhite);
    vColor.rgb /= (1.0f + vColor);

	//return vColor;
	//return pow(vColor, 1 / 2.2f);
	return pow( vColor, 2.2f );
} 

float3 ToneMap_jafEq4(float3 vColor, Texture2D lumTex, SamplerState samplerState)
{
	// Get the calculated average luminance
	float fLumAvg = lumTex.SampleLevel(samplerState, float2(0.5f, 0.5f), 9).r;

	// Calculate the luminance of the current pixel
	float fLumPixel = dot(vColor, LUM_CONVERT);
	
	// Apply the modified operator (Eq. 4)
	float fLumScaled = (fLumPixel * HDR_MiddleGray) / fLumAvg;
	float fLumCompressed = (fLumScaled * (1 + (fLumScaled / (HDR_LumWhite * HDR_LumWhite)))) / (1 + fLumScaled);
	return pow(fLumCompressed * vColor, 1/2.2f); 
	//return pow( vColor, 2.2f );
} 

static const float A = 0.15;
static const float B = 0.50;
static const float C = 0.10;
static const float D = 0.20;
static const float E = 0.02;
static const float F = 0.30;
static const float W = 11.2;

float3 Uncharted2TonemapOperator(float3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 Uncharted2Tonemap(float3 vColor, Texture2D lumTex, SamplerState samplerState) : COLOR
{
	float fLumAvg = lumTex.SampleLevel(samplerState, float2(0.5f, 0.5f), 9).r;
	
	vColor *= HDR_MiddleGray / fLumAvg;  // Exposure Adjustment

	float ExposureBias = 2.0f;
	float3 curr = Uncharted2TonemapOperator(ExposureBias*vColor);	
	
	float3 whiteScale = 1.0f/Uncharted2TonemapOperator(HDR_LumWhite);
	float3 color = curr*whiteScale;

	float3 retColor = color;//pow(color,1/2.2f);
	return retColor;
}

float3 ToneMap_Simple(float3 vColor, Texture2D lumTex, SamplerState samplerState)
{
	// Get the calculated average luminance
	float fLumAvg = lumTex.SampleLevel(samplerState, float2(0.5f, 0.5f), 9).r;
	
	vColor *= HDR_MiddleGray/(fLumAvg + 0.001f);
	vColor /= (1.0f + vColor);
	
	return vColor;
}


//#define ToneMap ToneMap_jafEq4
//#define ToneMap Uncharted2Tonemap
#define ToneMap ToneMap_Simple

#endif