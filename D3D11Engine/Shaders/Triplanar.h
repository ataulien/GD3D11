
static const float WEIGHT_BIAS = -0.55;
static const float WEIGHT_MUL = 0.7;

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
float3 perturb_normal( float3 N, float3 V, Texture2D tex, float2 texcoord, SamplerState samplerState, float normalmapDepth = 1.0f)
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
	
#ifdef TRI_SAMPLE_LEVEL
    float3 nrmmap = tex.SampleLevel(samplerState, texcoord, TRI_SAMPLE_LEVEL).xyz * 2 - 1;
#else
	float3 nrmmap = tex.Sample(samplerState, texcoord).xyz * 2 - 1;
#endif

	nrmmap.xy *= -1.0f;
	nrmmap.xy *= normalmapDepth;
	nrmmap = normalize(nrmmap);
	
    float3x3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( mul(transpose(TBN), nrmmap) );
}

float3 GetTriPlanarNrmMap(Texture2D yzTexture,Texture2D zxTexture,Texture2D xyTexture, SamplerState samplerState, float2 TexScale, float NrmMapDepth, float3 VertexPos, float3 Normals, float3 vsPos, float3 vsNrm)
{
		// Determine the blend weights for the 3 planar projections.
	// N_orig is the vertex-interpolated normal vector.
	float3 blend_weights = abs( Normals );

	// Tighten up the blending zone:
	blend_weights = (blend_weights +WEIGHT_BIAS) * WEIGHT_MUL;
	blend_weights = max(blend_weights, 0);

	// Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y +
	blend_weights.z ).xxx;

	// Now determine a color value and bump vector for each of the 3
	// projections, blend them, and store blended results in these two
	// vectors:
	float4 blended_color;  // .w hold spec value
	float3 blended_bump_vec;
	
	// Compute the UV coords for each of the 3 planar projections.
	// tex_scale (default ~ 1.0) determines how big the textures appear.
	float2 coord1 = (VertexPos.yz * TexScale).yx;
	float2 coord2 = (VertexPos.zx * TexScale).yx;
	float2 coord3 = (VertexPos.xy * TexScale).xy;

	// This is where you would apply conditional displacement mapping.
	//if (blend_weights.x > 0) coord1 = . . .
	//if (blend_weights.y > 0) coord2 = . . .
	//if (blend_weights.z > 0) coord3 = . . .	

	// Sample bump maps too, and generate bump vectors.
	float3 bump1 = perturb_normal(vsNrm, vsPos, yzTexture, coord1, samplerState, NrmMapDepth);
	float3 bump2 = perturb_normal(vsNrm, vsPos, zxTexture, coord2, samplerState, NrmMapDepth);
	float3 bump3 = perturb_normal(vsNrm, vsPos, xyTexture, coord3, samplerState, NrmMapDepth);

	blended_bump_vec = bump1.xyz * blend_weights.xxx +
				     bump2.xyz * blend_weights.yyy +
				     bump3.xyz * blend_weights.zzz;

	return blended_bump_vec;
}

float4 GetTriPlanarTexture(Texture2D yzTexture,Texture2D zxTexture,Texture2D xyTexture, SamplerState samplerState, float2 TexScale, float3 VertexPos, float3 Normals)
{
	// Determine the blend weights for the 3 planar projections.
	// N_orig is the vertex-interpolated normal vector.
	float3 blend_weights = abs( Normals );

	// Tighten up the blending zone:
	blend_weights = (blend_weights +WEIGHT_BIAS) * WEIGHT_MUL;
	blend_weights = max(blend_weights, 0);

	// Force weights to sum to 1.0 (very important!)
	blend_weights /= (blend_weights.x + blend_weights.y +
	blend_weights.z ).xxx;

	// Now determine a color value and bump vector for each of the 3
	// projections, blend them, and store blended results in these two
	// vectors:
	float4 blended_color;  // .w hold spec value
	float3 blended_bump_vec;
	
	// Compute the UV coords for each of the 3 planar projections.
	// tex_scale (default ~ 1.0) determines how big the textures appear.
	float2 coord1 = (VertexPos.yz * TexScale).yx;
	float2 coord2 = (VertexPos.zx * TexScale).yx;
	float2 coord3 = (VertexPos.xy * TexScale).xy;

	// This is where you would apply conditional displacement mapping.
	//if (blend_weights.x > 0) coord1 = . . .
	//if (blend_weights.y > 0) coord2 = . . .
	//if (blend_weights.z > 0) coord3 = . . .

	// Sample color maps for each projection, at those UV coords.
	
#ifdef TRI_SAMPLE_LEVEL
	float4 col1 = yzTexture.SampleLevel(samplerState, coord1, TRI_SAMPLE_LEVEL);
	float4 col2 = zxTexture.SampleLevel(samplerState, coord2, TRI_SAMPLE_LEVEL);
	float4 col3 = xyTexture.SampleLevel(samplerState, coord3, TRI_SAMPLE_LEVEL);
#else
	float4 col1 = yzTexture.Sample(samplerState, coord1);
	float4 col2 = zxTexture.Sample(samplerState, coord2);
	float4 col3 = xyTexture.Sample(samplerState, coord3);
#endif

	// Finally, blend the results of the 3 planar projections.
	blended_color = col1.xyzw * blend_weights.xxxx +
				  col2.xyzw * blend_weights.yyyy +
				  col3.xyzw * blend_weights.zzzz;

	return blended_color;
}