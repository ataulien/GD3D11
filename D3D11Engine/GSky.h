#pragma once
#include "pch.h"
#include "WorldConverter.h"
#include "ConstantBufferStructs.h"

struct AtmosphereSettings
{
	float G;
	float Kr;
	float Km;
	float ESun;
	float InnerRadius;
	float OuterRadius;
	int Samples;
	float RayleightScaleDepth;
	float3 WaveLengths;
	D3DXVECTOR3 SpherePosition;
	float SphereOffsetY;
	D3DXVECTOR3 LightDirection;
	float SkyTimeScale;
};

enum ESkyTexture
{
	ST_NewWorld,
	ST_OldWorld
};

class zCSkyLayer;
class zCSkyState;
class GMesh;
class BaseTexture;
class GSky
{
public:
	GSky(void);
	~GSky(void);

	/** Creates needed resources by the sky */
	XRESULT InitSky();

	/** Renders the sky */
	XRESULT RenderSky();

	/** Returns the sky-texture for the passed daytime (0..1) */
	void GetTextureOfDaytime(float time, BaseTexture** t1, BaseTexture** t2, float* factor);

	/** Returns the loaded sky-Dome */
	GMesh* GetSkyDome();

	/** Sets the current sky texture */
	void SetSkyTexture(ESkyTexture texture);

	/** Returns the skyplane */
	MeshInfo* GetSkyPlane();

	/** Returns the cloud meshes */
	std::vector<GMesh *>& GetCloudMeshes();

	/** Returns the atmospheric parameters */
	AtmosphereConstantBuffer& GetAtmosphereCB(){return AtmosphereCB;}

	/** returns atmosphere settings */
	AtmosphereSettings& GetAtmoshpereSettings(){return Atmosphere;}

	/** Returns the current sky-light color */
	float4 GetSkylightColor();

	/** Returns the cloud texture */
	BaseTexture* GetCloudTexture();

	/** Returns the night texture */
	BaseTexture* GetNightTexture();

	/** Returns the current sun color */
	float3 GetSunColor();
protected:

	/** Loads the sky textures */
	XRESULT LoadSkyResources();

	/** Adds a sky texture. Sky textures must be in order to make the daytime work */
	XRESULT AddSkyTexture(const std::string& file);

	/** Adds a cloud mesh */
	XRESULT AddCloudMesh(const std::string& file);

	/** Renders a sky layer */
	XRESULT RenderSkyLayer(zCSkyLayer* layer, zCSkyState* state);



	/** Sky mesh */
	GMesh* SkyDome;

	/** Skyplane */
	MeshInfo* SkyPlane;

	/** Cloud meshes */
	std::vector<GMesh *> CloudMeshes;

	/** Sky textures by time:
		1=0	0.25	0.5	  0.75	  1=0
		Day	Evening	Night Morning Day
	*/
	std::vector<BaseTexture*> SkyTextures;

	BaseTexture* CloudTexture;
	BaseTexture* NightTexture;

	BaseVertexBuffer* SkyPlaneVertexBuffer;
	ExVertexStruct SkyPlaneVertices[6];

	/** Atmospheric variables */
	AtmosphereConstantBuffer AtmosphereCB;
	AtmosphereSettings Atmosphere;
};

