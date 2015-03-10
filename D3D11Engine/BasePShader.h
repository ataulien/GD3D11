#pragma once
#include "pch.h"

class BaseConstantBuffer;
class BasePShader
{
public:
	BasePShader(void);
	virtual ~BasePShader(void);

	/** Loads shader */
	virtual XRESULT LoadShader(const char* pixelShader, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>()) = 0;

	/** Applys the shader */
	virtual XRESULT Apply() = 0;

	/** Returns a reference to the constantBuffer vector*/
	virtual std::vector<BaseConstantBuffer*>& GetConstantBuffer() = 0;
};

