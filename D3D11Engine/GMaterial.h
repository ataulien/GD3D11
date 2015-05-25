#pragma once

class zCMaterial;
class BaseTexture;
class GMaterial
{
public:
	GMaterial(zCMaterial* sourceMaterial);
	~GMaterial(void);

	/** Returns the currently active texture on this */
	BaseTexture* GetTexture();
protected:
	/** Source material this operates on */
	zCMaterial* SourceMaterial;
};

