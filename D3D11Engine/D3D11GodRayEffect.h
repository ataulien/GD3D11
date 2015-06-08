#pragma once

class D3D11PShader;
class D3D11VShader;
class D3D11HDShader;
class D3D11GodRayEffect
{
public:
	D3D11GodRayEffect(void);
	~D3D11GodRayEffect(void);

	D3D11VShader* QuadVS;
	D3D11PShader* QuadPS;
	D3D11HDShader* QuadHDS;
};

