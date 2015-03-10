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

#include "../pch.h"


//Memory access coherency (in threads)
#define COHERENCY_GRANULARITY 128


///////////////////////////////////////////////////////////////////////////////
// Common types
///////////////////////////////////////////////////////////////////////////////

typedef struct CSFFT_512x512_Data_t
{
	// D3D11 objects
	ID3D11DeviceContext* pd3dImmediateContext;
	ID3D11ComputeShader* pRadix008A_CS;
	ID3D11ComputeShader* pRadix008A_CS2;

	// More than one array can be transformed at same time
	UINT slices;

	// For 512x512 config, we need 6 constant buffers
	ID3D11Buffer* pRadix008A_CB[6];

	// Temporary buffers
	ID3D11Buffer* pBuffer_Tmp;
	ID3D11UnorderedAccessView* pUAV_Tmp;
	ID3D11ShaderResourceView* pSRV_Tmp;
} CSFFT512x512_Plan;

////////////////////////////////////////////////////////////////////////////////
// Common constants
////////////////////////////////////////////////////////////////////////////////
#define TWO_PI 6.283185307179586476925286766559

#define FFT_DIMENSIONS 3U
#define FFT_PLAN_SIZE_LIMIT (1U << 27)

#define FFT_FORWARD -1
#define FFT_INVERSE 1


void fft512x512_create_plan(CSFFT512x512_Plan* plan, ID3D11Device* pd3dDevice, UINT slices);
void fft512x512_destroy_plan(CSFFT512x512_Plan* plan);

void fft_512x512_c2c(CSFFT512x512_Plan* fft_plan, 
					 ID3D11UnorderedAccessView* pUAV_Dst,
					 ID3D11ShaderResourceView* pSRV_Dst,
					 ID3D11ShaderResourceView* pSRV_Src);
