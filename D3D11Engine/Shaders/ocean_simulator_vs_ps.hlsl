// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

//---------------------------------------- Vertex Shaders ------------------------------------------
struct VS_QUAD_OUTPUT
{
    float4 Position		: SV_POSITION;	// vertex position
    float2 TexCoord		: TEXCOORD0;	// vertex texture coords 
};

VS_QUAD_OUTPUT QuadVS(float4 vPos : POSITION)
{
	VS_QUAD_OUTPUT Output;

	Output.Position = vPos;
	Output.TexCoord.x = 0.5f + vPos.x * 0.5f;
	Output.TexCoord.y = 0.5f - vPos.y * 0.5f;

	return Output;
}

//----------------------------------------- Pixel Shaders ------------------------------------------

// Textures and sampling states
Texture2D g_samplerDisplacementMap : register(t0);

SamplerState LinearSampler : register(s0);

// Constants
cbuffer cbImmutable : register(b0)
{
	uint g_ActualDim;
	uint g_InWidth;
	uint g_OutWidth;
	uint g_OutHeight;
	uint g_DxAddressOffset;
	uint g_DyAddressOffset;
};

cbuffer cbChangePerFrame : register(b1)
{
	float g_Time;
	float g_ChoppyScale;
	float g_GridLen;
};

// The following three should contains only real numbers. But we have only C2C FFT now.
StructuredBuffer<float2>	g_InputDxyz		: register(t0);


// Post-FFT data wrap up: Dx, Dy, Dz -> Displacement
float4 UpdateDisplacementPS(VS_QUAD_OUTPUT In) : SV_Target
{
	uint index_x = (uint)(In.TexCoord.x * (float)g_OutWidth);
	uint index_y = (uint)(In.TexCoord.y * (float)g_OutHeight);
	uint addr = g_OutWidth * index_y + index_x;

	// cos(pi * (m1 + m2))
	int sign_correction = ((index_x + index_y) & 1) ? -1 : 1;

	float dx = g_InputDxyz[addr + g_DxAddressOffset].x * sign_correction * g_ChoppyScale;
	float dy = g_InputDxyz[addr + g_DyAddressOffset].x * sign_correction * g_ChoppyScale;
	float dz = g_InputDxyz[addr].x * sign_correction;

	return float4(dx, dy, dz, 1);
}

// Displacement -> Normal, Folding
float4 GenGradientFoldingPS(VS_QUAD_OUTPUT In) : SV_Target
{
	// Sample neighbour texels
	float2 one_texel = float2(1.0f / (float)g_OutWidth, 1.0f / (float)g_OutHeight);

	float2 tc_left  = float2(In.TexCoord.x - one_texel.x, In.TexCoord.y);
	float2 tc_right = float2(In.TexCoord.x + one_texel.x, In.TexCoord.y);
	float2 tc_back  = float2(In.TexCoord.x, In.TexCoord.y - one_texel.y);
	float2 tc_front = float2(In.TexCoord.x, In.TexCoord.y + one_texel.y);

	float3 displace_left  = g_samplerDisplacementMap.Sample(LinearSampler, tc_left).xyz;
	float3 displace_right = g_samplerDisplacementMap.Sample(LinearSampler, tc_right).xyz;
	float3 displace_back  = g_samplerDisplacementMap.Sample(LinearSampler, tc_back).xyz;
	float3 displace_front = g_samplerDisplacementMap.Sample(LinearSampler, tc_front).xyz;
	
	// Do not store the actual normal value. Using gradient instead, which preserves two differential values.
	float2 gradient = {-(displace_right.z - displace_left.z), -(displace_front.z - displace_back.z)};
	

	// Calculate Jacobian corelation from the partial differential of height field
	float2 Dx = (displace_right.xy - displace_left.xy) * g_ChoppyScale * g_GridLen;
	float2 Dy = (displace_front.xy - displace_back.xy) * g_ChoppyScale * g_GridLen;
	float J = (1.0f + Dx.x) * (1.0f + Dy.y) - Dx.y * Dy.x;

	// Practical subsurface scale calculation: max[0, (1 - J) + Amplitude * (2 * Coverage - 1)].
	float fold = max(1.0f - J, 0);

	// Output
	return float4(gradient, 0, fold);
}
