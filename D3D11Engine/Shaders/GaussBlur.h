
static const float GAUSS_PixelKernel[13] =
{
    -6,
    -5,
    -4,
    -3,
    -2,
    -1,
     0,
     1,
     2,
     3,
     4,
     5,
     6
};

static const float GAUSS_BlurWeights[13] = 
{ 
	0.0048748912161282396, 
	0.0146449825619271, 
	0.03602084467215462, 
	0.072537073483924, 
	0.11959341596728278, 
	0.16143422587153644, 
	0.1784124116152771, 
	0.16143422587153644, 
	0.11959341596728278, 
	0.072537073483924, 
	0.03602084467215462, 
	0.0146449825619271, 
	0.0048748912161282396, 
};

float BlurSimple(float2 pixelSize, float2 texCoord, Texture2D tx, SamplerState ss, float blurSize = 1.0f)
{
	float4 c = 0;

	for(int x = 0; x < 13; x+=5) 
	{
		for(int y = 0; y < 13; y+=5) 
		{
			int2 px = int2(GAUSS_PixelKernel[x], GAUSS_PixelKernel[y]);
			
			int idx = (int)sqrt(x*x + y*y);
			float weight = GAUSS_BlurWeights[idx];
			
			float2 uv = texCoord + (px * pixelSize * blurSize);
			c += tx.Sample(ss, uv) * weight;
		}
    }
	
	return c;
}

float4 DoBlurPassSingle(float2 pixelSize, float2 texCoord, Texture2D tx, Texture2D depth, SamplerState ss, float blurSize = 1.0f)
{
	float4 c = 0;

	/*for(int x = 0; x < 13; x+=5) 
	{
		for(int y = 0; y < 13; y+=5) 
		{
			int2 px = int2(GAUSS_PixelKernel[x], GAUSS_PixelKernel[y]);
			
			int idx = (int)sqrt(x*x + y*y);
			float weight = GAUSS_BlurWeights[idx];
			
			float2 uv = texCoord + (px * pixelSize * blurSize);
			c += tx.Sample(ss, uv) * weight;
		}
    }*/
	
	float4 center = tx.Sample(ss, texCoord);
	float centerDepth = depth.Sample(ss, texCoord);
	
	int i=0;
	for(int x = -5; x < 5; x++) 
	{
		for(int y = -5; y < 5; y++) 
		{
			int2 px = int2(x,y);
						
			float2 uv = texCoord + (px * pixelSize * blurSize);
			
			float d = depth.Sample(ss, uv);
			
			if(centerDepth > d + 0.0018f)
				continue;
			else
				c += tx.Sample(ss, uv);
				
			i++;
		}
    }
	
	c /= i;

	return c;
}

float4 DoBlurPass(float2 pixelSize, float2 texCoord, Texture2D tx, SamplerState ss, float blurSize = 1.0f)
{
	float4 c = 0;

	for(int i = 0; i < 13; i++) 
	{
        float2 uv = texCoord + (GAUSS_PixelKernel[i] * pixelSize * blurSize);
        c += tx.Sample(ss, saturate(uv)) * GAUSS_BlurWeights[i];
    }

	return c;
}

float4 DoBlurPassXA(float2 pixelSize, float2 texCoord, Texture2D tx, SamplerState ss, float blurSize = 1.0f)
{
	float4 c = 0;

	for(int i = 0; i < 13; i++) 
	{
        float2 uv = texCoord + (GAUSS_PixelKernel[i] * pixelSize * blurSize);
		float4 sample = tx.Sample(ss, uv);
		sample.a = sample.a > 0 ? 1 : 0;
		
        c += sample * GAUSS_BlurWeights[i];
    }

	return c;
}
