#pragma once
#include "BasePShader.h"

class D3D11ConstantBuffer;

class D3D11PShader : public BasePShader
{
public:
	D3D11PShader(void);
	~D3D11PShader(void);

	/** Loads shader */
	XRESULT LoadShader(const char* pixelShader, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>());

	/** Applys the shader */
	XRESULT Apply();

	/** Returns a reference to the constantBuffer vector*/
	std::vector<BaseConstantBuffer*>& GetConstantBuffer();
private:

	/** Compiles the shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros);

	ID3D11PixelShader* PixelShader;
	std::vector<BaseConstantBuffer*> ConstantBuffers;

	std::string File;
};

