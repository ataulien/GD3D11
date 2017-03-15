#pragma once
#include "baseanttweakbar.h"
class D3D11AntTweakBar :
	public BaseAntTweakBar
{
public:
	D3D11AntTweakBar(void);
	~D3D11AntTweakBar(void);

	/** Creates the resources */
	XRESULT Init() override;
};

