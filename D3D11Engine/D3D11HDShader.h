#pragma once
#include "pch.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class D3D11HDShader
{
public:
	D3D11HDShader(void);
	~D3D11HDShader(void);

		/** Loads shader */
	XRESULT LoadShader(const char* hullShader, const char* domainShader);

	/** Applys the shader */
	XRESULT Apply();

	/** Unbinds the currently bound hull/domain shaders */
	static void Unbind();

	/** Returns a reference to the constantBuffer vector*/
	std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();
private:

	/** Compiles the shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	ID3D11HullShader* HullShader;
	ID3D11DomainShader* DomainShader;
	std::vector<D3D11ConstantBuffer*> ConstantBuffers;

	std::string File;
};

