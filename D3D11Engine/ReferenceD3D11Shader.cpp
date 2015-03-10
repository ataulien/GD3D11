#include "pch.h"
#include "ReferenceD3D11Shader.h"
#include "ReferenceD3D11GraphicsEngine.h"
#include <D3DX11.h>
#include "Engine.h"
#include "GothicAPI.h"

ReferenceD3D11Shader::ReferenceD3D11Shader(void)
{
	PixelShader = NULL;
	VertexShader = NULL;
	InputLayout = NULL;
}


ReferenceD3D11Shader::~ReferenceD3D11Shader(void)
{
	if(PixelShader)PixelShader->Release();
	if(VertexShader)VertexShader->Release();
	if(InputLayout)InputLayout->Release();
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT ReferenceD3D11Shader::CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
    hr = D3DX11CompileFromFileA( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
		LogInfo() << "Shader compilation failed!";
        if( pErrorBlob != NULL )
		{     	

			LogErrorBox() << (char*)pErrorBlob->GetBufferPointer() << "\n\n (You can ignore the next error from Gothic about too small video memory!)";
			pErrorBlob->Release();
		}

		SetCurrentDirectoryA(dir);
        return hr;
    }
    if(pErrorBlob)
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
XRESULT ReferenceD3D11Shader::LoadShaders(const char* vertexShader, const char* pixelShader, int layout)
{
	HRESULT hr;
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	LogInfo() << "Compilling shaders: " << vertexShader << ", " << pixelShader;

	// Compile shaders
	CompileShaderFromFile(vertexShader, "VSMain", "vs_4_0", &vsBlob);
	CompileShaderFromFile(pixelShader, "PSMain", "ps_4_0", &psBlob);

	// Create the shaders
	LE( engine->GetDevice()->CreateVertexShader( vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), NULL, &VertexShader ) );

	LE( engine->GetDevice()->CreatePixelShader( psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), NULL, &PixelShader ) );


	const D3D11_INPUT_ELEMENT_DESC layout1[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "DIFFUSE",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const D3D11_INPUT_ELEMENT_DESC layout3[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION",  1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION",  2, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION",  3, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "DIFFUSE",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEIDS", 0, DXGI_FORMAT_R8G8B8A8_UINT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	switch(layout)
	{
	case 1:
		LE( engine->GetDevice()->CreateInputLayout( layout1, ARRAYSIZE( layout1 ), vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(), &InputLayout ) );
		break;

	case 2:
		LE( engine->GetDevice()->CreateInputLayout( layout2, ARRAYSIZE( layout2 ), vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(), &InputLayout ) );
		break;

	case 3:
		LE( engine->GetDevice()->CreateInputLayout( layout3, ARRAYSIZE( layout3 ), vsBlob->GetBufferPointer(),
					vsBlob->GetBufferSize(), &InputLayout ) );
		break;
	}

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT ReferenceD3D11Shader::Apply()
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	engine->GetContext()->IASetInputLayout(InputLayout);
	engine->GetContext()->VSSetShader(VertexShader, NULL, 0);
	engine->GetContext()->PSSetShader(PixelShader, NULL, 0);

	return XR_SUCCESS;
}