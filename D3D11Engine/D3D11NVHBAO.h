#pragma once
#include "pch.h"

class GFSDK_SSAO_Context_D3D11;
class D3D11NVHBAO
{
public:
	D3D11NVHBAO(void);
	~D3D11NVHBAO(void);

	/** Initializes the library */
	XRESULT Init();

	/** Renders the HBAO-Effect onto the given RTV */
	XRESULT Render(ID3D11RenderTargetView* rtv);
private:
	/** Nvidia HBAO+ context */
	GFSDK_SSAO_Context_D3D11* AOContext;
};

