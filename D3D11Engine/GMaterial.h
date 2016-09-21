#pragma once

class zCMaterial;
class D3D11Texture;
class GMaterial
{
public:
	GMaterial(zCMaterial* sourceMaterial);
	~GMaterial(void);

	/** Returns the currently active texture on this */
	D3D11Texture* GetTexture();
protected:
	/** Source material this operates on */
	zCMaterial* SourceMaterial;
};

