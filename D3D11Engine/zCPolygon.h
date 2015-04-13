#pragma once
#include "pch.h"
#include "HookedFunctions.h"

#pragma pack (push, 1)	
#ifdef BUILD_GOTHIC_2_6_fix
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

#elif defined(BUILD_GOTHIC_1_08k)
struct PolyFlags 
{
		unsigned char PortalPoly : 2;	
		unsigned char Occluder : 1;		
		unsigned char SectorPoly : 1;		
		unsigned char LodFlag : 1;		
		unsigned char PortalIndoorOutdoor : 1;	
		unsigned char GhostOccluder : 1;		
		unsigned char NormalMainAxis : 2;	
		VERTEX_INDEX SectorIndex : 16;
};
#endif
#pragma pack (pop)

class zCVertex
{
public:
/*#ifdef BUILD_GOTHIC_1_08k
	int id;
#endif*/

	float3 Position;

	int TransformedIndex;
	int MyIndex;
};

class zCVertFeature
{
public:
	float3 normal;
	DWORD lightStatic;
	DWORD lightDynamic;
	float2 texCoord;
};


class zCTexture;
class zCMaterial;
class zCLightmap;
class zCPolygon
{
public:
	~zCPolygon()
	{
		// Clean our vertices
		for(int i=0;i<GetNumPolyVertices();i++)
		{
			delete getVertices()[i];
			getVertices()[i] = NULL;
		}

		Destructor();
	}

	void Destructor()
	{
		#ifndef BUILD_GOTHIC_2_6_fix
		XCALL(GothicMemoryLocations::zCPolygon::Destructor);
		#endif
	}

	void Constructor()
	{
		#ifndef BUILD_GOTHIC_2_6_fix
		XCALL(GothicMemoryLocations::zCPolygon::Constructor);
		#endif
	}

	void AllocVertPointers(int num)
	{
		#ifndef BUILD_GOTHIC_2_6_fix
		XCALL(GothicMemoryLocations::zCPolygon::AllocVerts);
		#endif
	}

	void CalcNormal()
	{
		#ifndef BUILD_GOTHIC_2_6_fix
		XCALL(GothicMemoryLocations::zCPolygon::CalcNormal);
		#endif
	}

	void AllocVertData()
	{
		for(int i=0;i<GetNumPolyVertices();i++)
		{
			getVertices()[i] = new zCVertex;
		}
	}

	zCVertex** getVertices() const
	{
		return *(zCVertex ***)(((char *)this) + GothicMemoryLocations::zCPolygon::Offset_VerticesArray);
	}

	zCVertFeature** getFeatures() const
	{
		return *(zCVertFeature ***)(((char *)this) + GothicMemoryLocations::zCPolygon::Offset_FeaturesArray);
	}
	
	unsigned char GetNumPolyVertices() const
	{
		return *(unsigned char*)THISPTR_OFFSET(GothicMemoryLocations::zCPolygon::Offset_NumPolyVertices);
	}

	PolyFlags* GetPolyFlags() const
	{
		return (PolyFlags*)THISPTR_OFFSET(GothicMemoryLocations::zCPolygon::Offset_PolyFlags);
	}

	zCMaterial* GetMaterial() const
	{
		return *(zCMaterial **)(((char *)this) + GothicMemoryLocations::zCPolygon::Offset_Material);
	}

	void SetMaterial(zCMaterial* material)
	{
		*(zCMaterial **)(((char *)this) + GothicMemoryLocations::zCPolygon::Offset_Material) = material;
	}

	zCLightmap* GetLightmap() const
	{
		return *(zCLightmap **)(((char *)this) + GothicMemoryLocations::zCPolygon::Offset_Lightmap);
	}
	
	char data[56];	
};