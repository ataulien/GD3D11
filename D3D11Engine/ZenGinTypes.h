#pragma once
#include "pch.h"
#include "GothicMemoryLocations.h"



/*
enum zTResourceCacheState {
	zRES_FAILURE	=-1,
	zRES_CACHED_OUT	= 0,
	zRES_QUEUED		= 1,
	zRES_LOADING	= 2,
	zRES_CACHED_IN	= 3
};

class zCTexture;

class zCResourceManager
{
public:
};



class zCTexture
{
public:
	const char* GetName()
	{
		return __GetName().ToChar();
	}

	const char* GetNameWithoutExt()
	{
		std::string n = GetName();
		n.resize(n.size() - 4);

		return n.c_str();
	}

	void TouchTimeStamp()
	{
		XCALL(0x005DC810);
	}

	void TouchTimeStampLocal()
	{
		XCALL(0x005DC890);
	}

private:
	const zString& __GetName()
	{
		XCALL(GADDR::zCTexture_GetName);
	}
};

class zCMaterial
{
public:
};

#pragma pack (push, 1)	
struct PolyFlags {
	unsigned char PortalPoly			: 2;
	unsigned char Occluder				: 1;
	unsigned char SectorPoly			: 1;
	unsigned char MustRelight			: 1;
	unsigned char PortalIndoorOutdoor	: 1;	
	unsigned char GhostOccluder			: 1;
	unsigned char NoDynLightNear		: 1;	
	VERTEX_INDEX SectorIndex			: 16;
};
#pragma pack (pop)

class zCVertex
{
public:
	float3 Position;
};

class zCVertFeature
{
public:
	float3 normal;
	DWORD lightStatic;
	DWORD lightDynamic;
	float2 texCoord;
};

class zCPolygon
{
public:
	float3** getVertices() const
	{
		return *(float3 ***)(((char *)this) + GADDR::Offset_zCPolygon::Offset_VerticesArray);
	}

	zCVertFeature** getFeatures() const
	{
		return *(zCVertFeature ***)(((char *)this) + GADDR::Offset_zCPolygon::Offset_FeaturesArray);
	}
	
	unsigned char GetNumPolyVertices() const
	{
		return *(unsigned char*)THISPTR_OFFSET(GADDR::Offset_zCPolygon::Offset_NumPolyVertices);
	}

	PolyFlags* GetPolyFlags() const
	{
		return (PolyFlags*)THISPTR_OFFSET(GADDR::Offset_zCPolygon::Offset_PolyFlags);
	}

	zCMaterial* GetMaterial() const
	{
		return *(zCMaterial **)(((char *)this) + GADDR::Offset_zCPolygon::Offset_Material);
	}

	zCTexture* GetLightmap() const
	{
		return *(zCTexture **)(((char *)this) + GADDR::Offset_zCPolygon::Offset_Lightmap);
	}
};

template <class T>
class zCArray
{
public:
	T Get(unsigned int idx, unsigned int size = sizeof(T)) const
	{
		return Array[idx];
	}

	T* Array;
	int		numAlloc;
	int		numInArray;
};
*/