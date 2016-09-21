#include "pch.h"
#include "GMaterial.h"
#include "zCMaterial.h"

GMaterial::GMaterial(zCMaterial* sourceMaterial)
{
	SourceMaterial = sourceMaterial;
}


GMaterial::~GMaterial(void)
{
}

/** Returns the currently active texture on this */
D3D11Texture* GMaterial::GetTexture()
{
	if(SourceMaterial->GetTexture() && SourceMaterial->GetTexture()->GetSurface())
		return SourceMaterial->GetTexture()->GetSurface()->GetEngineTexture();

	return NULL;
}