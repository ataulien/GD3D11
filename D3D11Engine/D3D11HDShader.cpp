#include "pch.h"
#include "D3D11HDShader.h"
#include "D3D11GraphicsEngine.h"
#include <D3DX11.h>
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"

// Patch HLSL-Compiler for http://support.microsoft.com/kb/2448404
#if D3DX_VERSION == 0xa2b
#pragma ruledisable 0x0802405f
#endif

D3D11HDShader::D3D11HDShader(void)
{
	HullShader = NULL;
	DomainShader = NULL;
	ConstantBuffers = std::vector<D3D11ConstantBuffer*>();
}


D3D11HDShader::~D3D11HDShader(void)
{
	if(HullShader)HullShader->Release();
	if(DomainShader)DomainShader->Release();

	for (unsigned int i = 0; i < ConstantBuffers.size(); i++)
	{
		delete ConstantBuffers[i];
	}
}


//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11HDShader::CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	char dir[260];
	GetCurrentDirectoryA(260, dir);
	SetCurrentDirectoryA(Engine::GAPI->GetStartDirectory().c_str());

	DWORD dwShaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	//dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFileA(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		LogInfo() << "Shader compilation failed!";
		if (pErrorBlob != NULL)
		{

			LogErrorBox() << (char*)pErrorBlob->GetBufferPointer() << "\n\n (You can ignore the next error from Gothic about too small video memory!)";
			pErrorBlob->Release();
		}

		SetCurrentDirectoryA(dir);
		return hr;
	}
	if (pErrorBlob)
	{
		/*if(Engine->SwapchainCreated())
		Engine->GetConsole()->PostConsoleMessage((char*)pErrorBlob->GetBufferPointer());
		else
		LogWarnBox() << (char*)pErrorBlob->GetBufferPointer() << "\n\n (You can ignore the next error from Gothic about too small video memory!)";
		*/
		pErrorBlob->Release();
	}

	SetCurrentDirectoryA(dir);
	return S_OK;
}

/** Loads both shaders at the same time */
XRESULT D3D11HDShader::LoadShader(const char* hullShader, const char* domainShader)
{
	HRESULT hr;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	ID3DBlob* hsBlob;
	ID3DBlob* dsBlob;

	LogInfo() << "Compilling hull shader: " << hullShader;
	File = hullShader;

	// Compile shaders
	if(FAILED(CompileShaderFromFile(hullShader, "HSMain", "hs_5_0", &hsBlob)))
	{
		return XR_FAILED;
	}

	if(FAILED(CompileShaderFromFile(domainShader, "DSMain", "ds_5_0", &dsBlob)))
	{
		return XR_FAILED;
	}

	// Create the shaders
	LE(engine->GetDevice()->CreateHullShader(hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), NULL, &HullShader));
	LE(engine->GetDevice()->CreateDomainShader(dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), NULL, &DomainShader));

#ifndef PUBLIC_RELEASE
	HullShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(hullShader), hullShader);
	DomainShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(domainShader), domainShader);
#endif

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11HDShader::Apply()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	engine->GetContext()->HSSetShader(HullShader, NULL, 0);
	engine->GetContext()->DSSetShader(DomainShader, NULL, 0);

	return XR_SUCCESS;
}

/** Unbinds the currently bound hull/domain shaders */
void D3D11HDShader::Unbind()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	engine->GetContext()->HSSetShader(NULL, NULL, 0);
	engine->GetContext()->DSSetShader(NULL, NULL, 0);
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11HDShader::GetConstantBuffer()
{
	return ConstantBuffers;
}