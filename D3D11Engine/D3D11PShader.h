#pragma once

class D3D11ConstantBuffer;

class D3D11PShader
{
public:

	D3D11PShader(void);
	~D3D11PShader(void);

	/** Loads shader */
	XRESULT LoadShader(const char* pixelShader, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>());

	/** Applys the shader */
	XRESULT Apply();

	/** Returns a reference to the constantBuffer vector*/
	std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();

	/** Returns the shader */
	ID3D11PixelShader* GetShader(){return PixelShader;}

	/** Returns this textures ID */
	UINT16 GetID(){return ID;};

private:

	/** Compiles the shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros);

	ID3D11PixelShader* PixelShader;
	std::vector<D3D11ConstantBuffer*> ConstantBuffers;

	std::string File;

	/** ID of this shader */
	UINT16 ID;
};

