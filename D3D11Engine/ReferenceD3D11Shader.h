#pragma once
#include "pch.h"

/** ** ** ** ** ** ** ** */
// This class is ugly design, it is only used for quick prototyping!
/** ** ** ** ** ** ** ** */

class ReferenceD3D11Shader
{
public:
	ReferenceD3D11Shader(void);
	~ReferenceD3D11Shader(void);

	/** Loads both shaders at the same time */
	XRESULT LoadShaders(const char* vertexShader, const char* pixelShader, int layout = 1);

	/** Applys the shaders */
	XRESULT Apply();
private:

	/** Compiles a shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	ID3D11PixelShader* PixelShader;
	ID3D11VertexShader* VertexShader;
	ID3D11InputLayout* InputLayout;
};

