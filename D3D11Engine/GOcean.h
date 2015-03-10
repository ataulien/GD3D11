#pragma once
#include "pch.h"

const int OCEAN_PATCH_SIZE = 2048;
const float OCEAN_DEFAULT_WAVE_HEIGHT = 1.0f;
const float OCEAN_DEFAULT_PATCH_HEIGHT = -700.0f;

struct WaterPatchInfo
{
	WaterPatchInfo()
	{
		WaveHeight = OCEAN_DEFAULT_WAVE_HEIGHT;
		PatchHeight = OCEAN_DEFAULT_PATCH_HEIGHT;
	}

	float WaveHeight;
	float PatchHeight;
};

class GMesh;
class OceanSimulator;
struct OceanSettingsConstantBuffer;
class GOcean
{
public:
	GOcean(void);
	virtual ~GOcean(void);

	/** Initializes the ocean */
	XRESULT InitOcean();

	/** Draws the ocean */
	void Draw();

	/** Returnst the planemesh for this ocean */
	GMesh* GetPlaneMesh(){return PlaneMesh;}

	/** Gets the oceans fft resources. Does not do addref() */
	void GetFFTResources(ID3D11ShaderResourceView** tex_displacement, ID3D11ShaderResourceView** tex_gradient, ID3D11ShaderResourceView** fresnelMap, OceanSettingsConstantBuffer* settingsCB);

	/** Adds a patch at the given location */
	WaterPatchInfo& AddWaterPatchAt(int x, int y);

	/** Returns a vector of the patch locations */
	void GetPatchLocations(std::vector<D3DXVECTOR3>& patchLocations);

	/** Clears all patches */
	void ClearPatches();

	/** Saves the patches to a file */
	XRESULT SavePatches(const std::string& file);

	/** Loads the patches from a file */
	XRESULT LoadPatches(const std::string& file);

protected:
	/** Creates the fresnel map */
	void CreateFresnelMap(ID3D11Device* pd3dDevice);

	/** Subdivided plane mesh for the ocean */
	GMesh* PlaneMesh;

	/** NVidias FFT-Ocean simulator */
	OceanSimulator* FFTOceanSimulator;

	/** Fresnel map */
	ID3D11ShaderResourceView* FresnelMapSRV;
	ID3D11Texture1D* FresnelMap;



	/** Map of where the waterpatches are */
	std::map<std::pair<int, int>, WaterPatchInfo> Patches;
};

