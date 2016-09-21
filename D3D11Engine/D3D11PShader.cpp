#include "pch.h"
#include "D3D11PShader.h"
#include "D3D11GraphicsEngineBase.h"
#include <D3DX11.h>
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"

D3D11PShader::D3D11PShader(void)
{
	PixelShader = NULL;

	// Insert into state-map
	ID = D3D11ObjectIDs::Counters.PShadersCounter++;

	D3D11ObjectIDs::PShadersByID[ID] = this;
}

D3D11PShader::~D3D11PShader(void)
{
	// Remove from state map
	Toolbox::EraseByElement(D3D11ObjectIDs::PShadersByID, this);

	if (PixelShader)PixelShader->Release();

	for (unsigned int i = 0; i < ConstantBuffers.size(); i++)
	{
		delete ConstantBuffers[i];
	}
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11PShader::CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros)
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

	// Construct makros
	std::vector<D3D10_SHADER_MACRO> m;
	D3D11GraphicsEngineBase::ConstructShaderMakroList(m);
	
	// Push these to the front
	m.insert(m.begin(), makros.begin(), makros.end());

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFileA(szFileName, &m[0], NULL, szEntryPoint, szShaderModel,
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
XRESULT D3D11PShader::LoadShader(const char* pixelShader, std::vector<D3D10_SHADER_MACRO>& makros)
{
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	ID3DBlob* psBlob;

	LogInfo() << "Compilling pixel shader: " << pixelShader;
	File = pixelShader;

	// Compile shaders
	if(FAILED(CompileShaderFromFile(pixelShader, "PSMain", "ps_4_0", &psBlob, makros)))
	{
		return XR_FAILED;
	}

	// Create the shader
	LE(engine->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &PixelShader));

#ifndef PUBLIC_RELEASE
	PixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(pixelShader), pixelShader);
#endif

	psBlob->Release();

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11PShader::Apply()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	engine->GetContext()->PSSetShader(PixelShader, NULL, 0);

	return XR_SUCCESS;
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11PShader::GetConstantBuffer()
{
	return ConstantBuffers;
}