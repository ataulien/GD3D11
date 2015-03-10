#pragma once
#include "pch.h"
#include <d3d11.h>
#include <D3DX10math.h>

struct SimpleVertexStruct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 TexCoord;
};

//This can draw a full screen quad
class D3D11FullscreenQuad
{
public:
	D3D11FullscreenQuad(void);
	virtual ~D3D11FullscreenQuad(void);

	//Fills the VertexBuffer
	HRESULT CreateQuad(ID3D11Device* device);

	ID3D11Buffer* GetBuffer(){return QuadVB;}

private:
	ID3D11Buffer* QuadVB; //Vertex buffer for the quad
};

