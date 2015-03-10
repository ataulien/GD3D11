#pragma once
#include "pch.h"

class BaseConstantBuffer;
class BaseVShader
{
public:
	BaseVShader(void);
	virtual ~BaseVShader(void);

	/** Loads both shader at the same time */
	virtual XRESULT LoadShader(const char* vertexShader, int layoput = 1, std::vector<D3D10_SHADER_MACRO>& makros = std::vector<D3D10_SHADER_MACRO>()) = 0;

	/** Applys the shader */
	virtual XRESULT Apply() = 0;

	/** Returns a reference to the constantBuffer vector*/
	virtual std::vector<BaseConstantBuffer*>& GetConstantBuffer() = 0;
};

