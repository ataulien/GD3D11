#pragma once
#include "pch.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;

class D3D11GShader
{
public:
	D3D11GShader(void);
	~D3D11GShader(void);

	
	/** Loads shader */
	XRESULT LoadShader(const char* geometryShader, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>(), bool createStreamOutFromVS = false, int soLayout = 0);

	/** Applys the shader */
	XRESULT Apply();

	/** Returns a reference to the constantBuffer vector*/
	std::vector<D3D11ConstantBuffer*>& GetConstantBuffer();

	/** Returns the shader */
	ID3D11GeometryShader* GetShader(){return GeometryShader;}

	/** Returns this textures ID */
	UINT16 GetID(){return ID;};

private:

	/** Compiles the shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros);

	ID3D11GeometryShader* GeometryShader;
	std::vector<D3D11ConstantBuffer*> ConstantBuffers;

	std::string File;

	/** ID of this shader */
	UINT16 ID;
};

