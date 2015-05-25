#pragma once
#include "pch.h"
#include "BaseVShader.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;

class D3D11VShader : public BaseVShader
{
public:
	D3D11VShader(void);
	~D3D11VShader(void);

	/** Loads both shader at the same time */
	XRESULT LoadShader(const char* vertexShader, int layoput = 1, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>());

	/** Applys the shader */
	XRESULT Apply();

	/** Returns a reference to the constantBuffer vector*/
	std::vector<BaseConstantBuffer*>& GetConstantBuffer();

	/** Returns the shader */
	ID3D11VertexShader* GetShader(){return VertexShader;}

	/** Returns the inputlayout */
	ID3D11InputLayout* GetInputLayout(){return InputLayout;}
	
	/** Returns this textures ID */
	UINT16 GetID(){return ID;};
private:

	/** Compiles a shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, std::vector<D3D10_SHADER_MACRO>& makros);

	ID3D11VertexShader* VertexShader;
	ID3D11InputLayout* InputLayout;
	std::vector<BaseConstantBuffer*> ConstantBuffers;

	std::string File;

	/** ID of this shader */
	UINT16 ID;
};

