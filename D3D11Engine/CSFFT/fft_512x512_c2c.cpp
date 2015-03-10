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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fft_512x512.h"

HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );


void radix008A(CSFFT512x512_Plan* fft_plan,
			   ID3D11UnorderedAccessView* pUAV_Dst,
			   ID3D11ShaderResourceView* pSRV_Src,
			   UINT thread_count,
			   UINT istride)
{
    // Setup execution configuration
	UINT grid = thread_count / COHERENCY_GRANULARITY;
	ID3D11DeviceContext* pd3dImmediateContext = fft_plan->pd3dImmediateContext;

	// Buffers
	ID3D11ShaderResourceView* cs_srvs[1] = {pSRV_Src};
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	ID3D11UnorderedAccessView* cs_uavs[1] = {pUAV_Dst};
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));

	// Shader
	if (istride > 1)
		pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS, NULL, 0);
	else
		pd3dImmediateContext->CSSetShader(fft_plan->pRadix008A_CS2, NULL, 0);

	// Execute
	pd3dImmediateContext->Dispatch(grid, 1, 1);

	// Unbind resource
	cs_srvs[0] = NULL;
	pd3dImmediateContext->CSSetShaderResources(0, 1, cs_srvs);

	cs_uavs[0] = NULL;
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, cs_uavs, (UINT*)(&cs_uavs[0]));
}

void fft_512x512_c2c(CSFFT512x512_Plan* fft_plan,
					 ID3D11UnorderedAccessView* pUAV_Dst,
					 ID3D11ShaderResourceView* pSRV_Dst,
					 ID3D11ShaderResourceView* pSRV_Src)
{
	const UINT thread_count = fft_plan->slices * (512 * 512) / 8;
	ID3D11UnorderedAccessView* pUAV_Tmp = fft_plan->pUAV_Tmp;
	ID3D11ShaderResourceView* pSRV_Tmp = fft_plan->pSRV_Tmp;
	ID3D11DeviceContext* pd3dContext = fft_plan->pd3dImmediateContext;
	ID3D11Buffer* cs_cbs[1];

	UINT istride = 512 * 512 / 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[0];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Src, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[1];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[2];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[3];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[4];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Tmp, pSRV_Dst, thread_count, istride);

	istride /= 8;
	cs_cbs[0] = fft_plan->pRadix008A_CB[5];
	pd3dContext->CSSetConstantBuffers(0, 1, &cs_cbs[0]);
	radix008A(fft_plan, pUAV_Dst, pSRV_Tmp, thread_count, istride);
}

void create_cbuffers_512x512(CSFFT512x512_Plan* plan, ID3D11Device* pd3dDevice, UINT slices)
{
	// Create 6 cbuffers for 512x512 transform.

	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = 32;//sizeof(float) * 5;
	cb_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cb_data;
	cb_data.SysMemPitch = 0;
	cb_data.SysMemSlicePitch = 0;

	struct CB_Structure
	{
		UINT thread_count;
		UINT ostride;
		UINT istride;
		UINT pstride;
		float phase_base;
	};

	// Buffer 0
	const UINT thread_count = slices * (512 * 512) / 8;
	UINT ostride = 512 * 512 / 8;
	UINT istride = ostride;
	double phase_base = -TWO_PI / (512.0 * 512.0);
	
	CB_Structure cb_data_buf0 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf0;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[0]);
	assert(plan->pRadix008A_CB[0]);

	// Buffer 1
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf1 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf1;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[1]);
	assert(plan->pRadix008A_CB[1]);

	// Buffer 2
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf2 = {thread_count, ostride, istride, 512, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf2;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[2]);
	assert(plan->pRadix008A_CB[2]);

	// Buffer 3
	istride /= 8;
	phase_base *= 8.0;
	ostride /= 512;
	
	CB_Structure cb_data_buf3 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf3;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[3]);
	assert(plan->pRadix008A_CB[3]);

	// Buffer 4
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf4 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf4;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[4]);
	assert(plan->pRadix008A_CB[4]);

	// Buffer 5
	istride /= 8;
	phase_base *= 8.0;
	
	CB_Structure cb_data_buf5 = {thread_count, ostride, istride, 1, (float)phase_base};
	cb_data.pSysMem = &cb_data_buf5;

	pd3dDevice->CreateBuffer(&cb_desc, &cb_data, &plan->pRadix008A_CB[5]);
	assert(plan->pRadix008A_CB[5]);
}

void fft512x512_create_plan(CSFFT512x512_Plan* plan, ID3D11Device* pd3dDevice, UINT slices)
{
	plan->slices = slices;

	// Context
	pd3dDevice->GetImmediateContext(&plan->pd3dImmediateContext);
	assert(plan->pd3dImmediateContext);

	// Compute shaders
    ID3DBlob* pBlobCS = NULL;
    ID3DBlob* pBlobCS2 = NULL;

    CompileShaderFromFile(L"CSFFT\\fft_512x512_c2c.hlsl", "Radix008A_CS", "cs_4_0", &pBlobCS);
    CompileShaderFromFile(L"CSFFT\\fft_512x512_c2c.hlsl", "Radix008A_CS2", "cs_4_0", &pBlobCS2);
	assert(pBlobCS);
	assert(pBlobCS2);

    pd3dDevice->CreateComputeShader(pBlobCS->GetBufferPointer(), pBlobCS->GetBufferSize(), NULL, &plan->pRadix008A_CS);
    pd3dDevice->CreateComputeShader(pBlobCS2->GetBufferPointer(), pBlobCS2->GetBufferSize(), NULL, &plan->pRadix008A_CS2);
	assert(plan->pRadix008A_CS);
	assert(plan->pRadix008A_CS2);
    
    SAFE_RELEASE(pBlobCS);
    SAFE_RELEASE(pBlobCS2);

	// Constants
	// Create 6 cbuffers for 512x512 transform
	create_cbuffers_512x512(plan, pd3dDevice, slices);

	// Temp buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = sizeof(float) * 2 * (512 * slices) * 512;
    buf_desc.Usage = D3D11_USAGE_DEFAULT;
    buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    buf_desc.CPUAccessFlags = 0;
    buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    buf_desc.StructureByteStride = sizeof(float) * 2;

	pd3dDevice->CreateBuffer(&buf_desc, NULL, &plan->pBuffer_Tmp);
	assert(plan->pBuffer_Tmp);

	// Temp undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = (512 * slices) * 512;
	uav_desc.Buffer.Flags = 0;

	pd3dDevice->CreateUnorderedAccessView(plan->pBuffer_Tmp, &uav_desc, &plan->pUAV_Tmp);
	assert(plan->pUAV_Tmp);

	// Temp shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = (512 * slices) * 512;

	pd3dDevice->CreateShaderResourceView(plan->pBuffer_Tmp, &srv_desc, &plan->pSRV_Tmp);
	assert(plan->pSRV_Tmp);
}

void fft512x512_destroy_plan(CSFFT512x512_Plan* plan)
{
	SAFE_RELEASE(plan->pSRV_Tmp);
	SAFE_RELEASE(plan->pUAV_Tmp);
	SAFE_RELEASE(plan->pBuffer_Tmp);
	SAFE_RELEASE(plan->pRadix008A_CS);
	SAFE_RELEASE(plan->pRadix008A_CS2);
	SAFE_RELEASE(plan->pd3dImmediateContext);

	for (int i = 0; i < 6; i++)
		SAFE_RELEASE(plan->pRadix008A_CB[i]);
}
