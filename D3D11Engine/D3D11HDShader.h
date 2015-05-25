#pragma once
#include "pch.h"

class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class D3D11HDShader
{
public:
	static std::map<UINT8, D3D11HDShader *> ShadersByID;

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

	/** Returns the shader */
	ID3D11HullShader* GetHShader(){return HullShader;}

	/** Returns the shader */
	ID3D11DomainShader* GetDShader(){return DomainShader;}
	
	/** Returns this textures ID */
	UINT16 GetID(){return ID;};
private:

	/** Compiles the shader from file and outputs error messages if needed */
	HRESULT CompileShaderFromFile(const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	ID3D11HullShader* HullShader;
	ID3D11DomainShader* DomainShader;
	std::vector<D3D11ConstantBuffer*> ConstantBuffers;

	std::string File;

	/** ID of this shader */
	UINT16 ID;
};

