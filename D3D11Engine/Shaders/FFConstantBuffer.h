#ifndef FFCONSTANTBUFFER_H
#define FFCONSTANTBUFFER_H
/** Constant buffer needed for the FF-Pipeline emulator */

static const int GSWITCH_FOG = 1;
static const int GSWITCH_ALPHAREF = 2;
static const int GSWITCH_LIGHING = 4;
static const int GSWITCH_REFLECTIONS = 8;

struct TextureStage
{
	int colorop;
	int colorarg1;
	int colorarg2;
	int alphaop;

	int alphaarg1;
	int alphaarg2;
	int2 pad;
};


// Values starting with FF_ for FixedFunction
cbuffer FFPipelineConstantBuffer : register( b0 )
{
	/** Fog section */
	float FF_FogWeight;
	float3 FF_FogColor;

	float FF_FogNear;
	float FF_FogFar;
	float FF_zNear;
	float FF_zFar;

	/** Lighting section */
	float3 FF_AmbientLighting;
	float FF_Time;

	/** Texture factor section */
	float4 FF_TextureFactor;

	/** Alpha ref section */
	float FF_AlphaRef;

	/** Graphical Switches (Takes GSWITCH_*) */
	unsigned int FF_GSwitches;
	float2 ggs_Pad3;
	
	TextureStage FF_Stages[2];
};

bool FF_AlphaTestEnabled()
{
	return (FF_GSwitches & GSWITCH_ALPHAREF) != 0;
}

void DoAlphaTest(float alpha)
{
	//if(FF_AlphaTestEnabled())
		clip(alpha - FF_AlphaRef);
}

#endif