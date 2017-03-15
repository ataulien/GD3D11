#include "pch.h"
#include "D3D11GShader.h"
#include "D3D11GraphicsEngineBase.h"
#include <D3DX11.h>
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"

D3D11GShader::D3D11GShader(void)
{
	GeometryShader = nullptr;

	// Insert into state-map
	ID = D3D11ObjectIDs::Counters.GShadersCounter++;

	D3D11ObjectIDs::GShadersByID[ID] = this;
}

D3D11GShader::~D3D11GShader(void)
{
	// Remove from state map
	Toolbox::EraseByElement(D3D11ObjectIDs::GShadersByID, this);

	if (GeometryShader)GeometryShader->Release();

	for (unsigned int i = 0; i < ConstantBuffers.size(); i++)
	{
		delete ConstantBuffers[i];
	}
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11GShader::CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros)
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
	hr = D3DX11CompileFromFileA(szFileName, &m[0], nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, nullptr, ppBlobOut, &pErrorBlob, nullptr);
	if (FAILED(hr))
	{
		LogInfo() << "Shader compilation failed!";
		if (pErrorBlob != nullptr)
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
XRESULT D3D11GShader::LoadShader(const char* geometryShader, std::vector<D3D10_SHADER_MACRO>& makros, bool createStreamOutFromVS, int soLayout)
{
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	ID3DBlob* gsBlob;

	LogInfo() << "Compiling geometry shader: " << geometryShader;
	File = geometryShader;

	if (!createStreamOutFromVS)
	{
		// Compile shaders
		if (FAILED(CompileShaderFromFile(geometryShader, "GSMain", "gs_4_0", &gsBlob, makros)))
		{
			return XR_FAILED;
		}

		// Create the shader
		LE(engine->GetDevice()->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), NULL, &GeometryShader));
	}
	else
	{
		// Compile vertexshader
		if (FAILED(CompileShaderFromFile(geometryShader, "VSMain", "vs_4_0", &gsBlob, makros)))
		{
			return XR_FAILED;
		}

		D3D11_SO_DECLARATION_ENTRY* soDec = nullptr;
		int numSoDecElements = 0;
		UINT stride = 0;

		struct output11
		{
			float4 vDiffuse;
			float3 vPosition;
			float2 vSize;
			float3 vVelocity;
			int type;
		};

		D3D11_SO_DECLARATION_ENTRY layout11[] =
		{
			{0, "POSITION", 0, 0, 3, 0},
			{0, "DIFFUSE", 0, 0, 4, 0},
			{0, "SIZE", 0, 0, 2, 0},
			{0, "TYPE", 0, 0, 1, 0},
			{0, "VELOCITY", 0, 0, 3, 0},

		};

		switch (soLayout)
		{
		case 11:
		default:
			soDec = layout11;
			numSoDecElements = sizeof(layout11) / sizeof(layout11[0]);
			stride = sizeof(output11);
			break;
		}

		// Create the shader from a vertexshader
		engine->GetDevice()->CreateGeometryShaderWithStreamOutput(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), soDec, numSoDecElements, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &GeometryShader);
	}
#ifndef PUBLIC_RELEASE
	GeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(geometryShader), geometryShader);
#endif

	gsBlob->Release();

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11GShader::Apply() const
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	engine->GetContext()->GSSetShader(GeometryShader, nullptr, 0);

	return XR_SUCCESS;
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11GShader::GetConstantBuffer()
{
	return ConstantBuffers;
}