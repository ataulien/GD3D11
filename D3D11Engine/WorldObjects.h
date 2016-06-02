#pragma once
#include "pch.h"
#include "GothicGraphicsState.h"
#include "BaseConstantBuffer.h"
#include "BaseTexture.h"
#include "zTypes.h"
#include "ConstantBufferStructs.h"
#include "zCPolygon.h"
#include "ShadowedPointLight.h"
#include "BaseVertexBuffer.h"

class zCMaterial;
class zCPolygon;
class BaseVertexBuffer;
class zCVob;
class zCTexture;
class zCLightmap;
struct zCModelNodeInst;
struct BspInfo;
class zCQuadMark;
struct MaterialInfo;

struct ParticleRenderInfo
{
	GothicBlendStateInfo BlendState;
	int BlendMode;
};

struct ParticleInstanceInfo
{
	float3 position;
	float4 color;
	float2 scale;
	int drawMode; // 0 = billboard, 1 = y-locked billboard, 2 = y-plane, 3 = velo aligned
	float3 velocity;
};

struct MeshKey
{
	zCTexture* Texture;
	zCMaterial* Material;
	MaterialInfo* Info;
	//zCLightmap* Lightmap;
};

struct cmpMeshKey {
    bool operator()(const MeshKey& a, const MeshKey& b) const 
	{
        return (a.Texture<b.Texture);
    }
};

/*struct MeshKey
{
	zCMaterial* Material;
	zCLightmap* Lightmap;
};

struct cmpMeshKey {
    bool operator()(const MeshKey& a, const MeshKey& b) const 
	{
        return (a.Material<b.Material) || (a.Material == b.Material && a.Lightmap<b.Lightmap);
    }
};*/

struct VisualTesselationSettings
{
	VisualTesselationSettings()
	{
		buffer.VT_DisplacementStrength = 0.0f;
		buffer.VT_Roundness = 1.0f;
		buffer.VT_TesselationFactor = 0.0f;
		Constantbuffer = NULL;
	}

	~VisualTesselationSettings()
	{
		delete Constantbuffer;
	}

	struct Buffer
	{
		float VT_TesselationFactor;
		float VT_Roundness;
		float VT_DisplacementStrength;
		float VT_Time;
	};

	/** creates/updates the constantbuffer */
	void UpdateConstantbuffer();

	BaseConstantBuffer* Constantbuffer;
	std::string TesselationShader;
	Buffer buffer;
};

/** Holds information about a mesh, ready to be loaded into the renderer */
struct MeshInfo
{
	MeshInfo()
	{
		MeshVertexBuffer = NULL;
		MeshIndexBuffer = NULL;
		BaseIndexLocation = 0;
		MeshIndexBufferPNAEN = NULL;

		WrappedVB = NULL;
		WrappedIB = NULL;
	}

	virtual ~MeshInfo();

	/** Creates buffers for this mesh info */
	XRESULT Create(ExVertexStruct* vertices, unsigned int numVertices, VERTEX_INDEX* indices, unsigned int numIndices);

	BaseVertexBuffer* MeshVertexBuffer;
	BaseVertexBuffer* MeshIndexBuffer;
	std::vector<ExVertexStruct> Vertices;
	std::vector<VERTEX_INDEX> Indices;	

	BaseVertexBuffer* MeshIndexBufferPNAEN;
	std::vector<VERTEX_INDEX> IndicesPNAEN;	
	std::vector<ExVertexStruct> VerticesPNAEN;
	unsigned int BaseIndexLocation;

	unsigned int VertexBufferOffset;
	unsigned int IndexBufferOffset;
	BaseVertexBuffer* WrappedVB;
	BaseVertexBuffer* WrappedIB;
};

struct WorldMeshInfo : public MeshInfo
{
	WorldMeshInfo()
	{
		SaveInfo = false;
	}

	/** Saves the info for this visual */
	void SaveWorldMeshInfo(const std::string& name);

	/** Loads the info for this visual */
	void LoadWorldMeshInfo(const std::string& name);

	VisualTesselationSettings TesselationSettings;

	/** If true we will save an info-file on next zen-resource-save */
	bool SaveInfo;
};

struct QuadMarkInfo
{
	QuadMarkInfo()
	{
		Mesh = NULL;
		NumVertices = 0;
	}

	~QuadMarkInfo()
	{
		delete Mesh;
	}

	BaseVertexBuffer* Mesh;
	int NumVertices;

	zCQuadMark* Visual;
	D3DXVECTOR3 Position;
};

/** Holds information about a skeletal mesh */
class zCMeshSoftSkin;
struct SkeletalMeshInfo
{
	SkeletalMeshInfo()
	{
		MeshVertexBuffer = NULL;
		MeshIndexBuffer = NULL;
		visual = NULL;
		MeshIndexBufferPNAEN = NULL;
	}

	~SkeletalMeshInfo();

	BaseVertexBuffer* MeshVertexBuffer;
	BaseVertexBuffer* MeshIndexBuffer;
	std::vector<ExSkelVertexStruct> Vertices;
	std::vector<VERTEX_INDEX> Indices;

	BaseVertexBuffer* MeshIndexBufferPNAEN;
	std::vector<VERTEX_INDEX> IndicesPNAEN;	

	/** Actual visual containing this */
	zCMeshSoftSkin* visual;
};

class zCVisual;
struct BaseVisualInfo
{
	BaseVisualInfo()
	{
		Visual = NULL;
	}

	virtual ~BaseVisualInfo()
	{
		for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
		{
			for(unsigned int i=0;i<(*it).second.size();i++)
				delete (*it).second[i];
		}
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	virtual void CreatePNAENInfo(bool softNormals = false){}

	/** Removes PNAEN info from this visual */
	virtual void ClearPNAENInfo();

	/** Saves the info for this visual */
	virtual void SaveMeshVisualInfo(const std::string& name);

	/** Loads the info for this visual */
	virtual void LoadMeshVisualInfo(const std::string& name);

	std::map<zCMaterial *, std::vector<MeshInfo*>> Meshes;

	/** Tesselation settings for this vob */
	VisualTesselationSettings TesselationInfo;

	/** "size" of the mesh. The distance between it's bbox min and bbox max */
	float MeshSize;

	/** Meshes bounding box */
	zTBBox3D BBox;

	/** Meshes midpoint */
	D3DXVECTOR3 MidPoint; 

	/** Games visual */
	zCVisual* Visual;

	/** Name of this visual */
	std::string VisualName;
};

/** Holds the converted mesh of a VOB */
class zCProgMeshProto;
class zCTexture;
struct MeshVisualInfo : public BaseVisualInfo
{
	MeshVisualInfo()
	{
		Visual = NULL;
		UnloadedSomething = false;
		StartInstanceNum = 0;
		FullMesh = NULL;
	}

	~MeshVisualInfo()
	{
		delete FullMesh;
	}

	/** Starts a new frame for this mesh */
	void StartNewFrame()
	{
		Instances.clear();
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	void CreatePNAENInfo(bool softNormals = false);

	std::map<MeshKey, std::vector<MeshInfo*>, cmpMeshKey> MeshesByTexture;

	// Vector of the MeshesByTexture-Map for faster access, since map iterations aren't Cache friendly
	std::vector<std::pair<MeshKey, std::vector<MeshInfo*>>> MeshesCached;

	//zCProgMeshProto* Visual;
	std::vector<VobInstanceInfo> Instances;
	unsigned int StartInstanceNum;

	/** Full mesh of this */
	MeshInfo* FullMesh;

	/** This is true if we can't actually render something on this. TODO: Try to fix this! */
	bool UnloadedSomething;
};

/** Holds the converted mesh of a VOB */
class zCMeshSoftSkin;
class zCModel;
struct SkeletalMeshVisualInfo : public BaseVisualInfo
{
	SkeletalMeshVisualInfo()
	{
		Visual = NULL;
	}

	~SkeletalMeshVisualInfo()
	{
		for(std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>>::iterator it = SkeletalMeshes.begin(); it != SkeletalMeshes.end(); it++)
		{
			for(unsigned int i=0;i<(*it).second.size();i++)
				delete (*it).second[i];
		}
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	void CreatePNAENInfo(bool softNormals = false);

	/** Removes PNAEN info from this visual */
	void ClearPNAENInfo();

	/** Submeshes of this visual */
	std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>> SkeletalMeshes;


};

struct BaseVobInfo
{
	virtual ~BaseVobInfo()
	{
	}
	/** Visual for this vob */
	BaseVisualInfo* VisualInfo;

	/** Vob the data came from */
	zCVob* Vob;
};

struct WorldMeshSectionInfo;
struct VobInfo : public BaseVobInfo
{
	VobInfo()
	{
		//Vob = NULL;
		VobConstantBuffer = NULL;
		IsIndoorVob = false;
		VisibleInRenderPass = false;
		VobSection = NULL;
	}

	~VobInfo()
	{
		//delete VisualInfo;
		delete VobConstantBuffer;
	}

	/** Updates the vobs constantbuffer */
	void UpdateVobConstantBuffer();

	/** Constantbuffer which holds this vobs world matrix */
	BaseConstantBuffer* VobConstantBuffer;

	/** Position the vob was at while being rendered last time */
	D3DXVECTOR3 LastRenderPosition;

	/** True if this is an indoor-vob */
	bool IsIndoorVob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** Section this vob is in */
	WorldMeshSectionInfo* VobSection;

	/** Current world transform */
	D3DXMATRIX WorldMatrix;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;

	/** Color the underlaying polygon has */
	DWORD GroundColor;
};

class zCVobLight;
class ShadowedPointLight;
struct VobLightInfo
{
	VobLightInfo()
	{
		Vob = NULL;
		LightShadowBuffers = NULL;
		VisibleInRenderPass = false;
		IsIndoorVob = false;
		DynamicShadows = false;
		UpdateShadows = true;
	}

	~VobLightInfo()
	{
		delete LightShadowBuffers;
	}

	/** Vob the data came from */
	zCVobLight* Vob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** True if this is an indoor-vob */
	bool IsIndoorVob;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;

	/** Buffers for doing shadows on this light */
	ShadowedPointLight* LightShadowBuffers;
	bool DynamicShadows; // Whether this light should be able to have dynamic shadows
	bool UpdateShadows; // Whether to update this lights shadows on the next occasion

	/** Position where we were rendered the last time */
	D3DXVECTOR3 LastRenderedPosition;
};


/** Holds the converted mesh of a VOB */
struct SkeletalVobInfo : public BaseVobInfo
{
	SkeletalVobInfo()
	{
		Vob = NULL;
		VisualInfo = NULL;
		IndoorVob = false;
		VisibleInRenderPass = false;
		VobConstantBuffer = NULL;
	}

	~SkeletalVobInfo()
	{
		//delete VisualInfo;
		
		for(std::map<int, std::vector<MeshVisualInfo *>>::iterator it = NodeAttachments.begin(); it != NodeAttachments.end(); it++)
		{
			for(unsigned int i=0;i<(*it).second.size();i++)
				delete (*it).second[i];
		}

		delete VobConstantBuffer;
	}

	/** Updates the vobs constantbuffer */
	void UpdateVobConstantBuffer();

	/** Constantbuffer which holds this vobs world matrix */
	BaseConstantBuffer* VobConstantBuffer;

	/** Map of visuals attached to nodes */
	std::map<int, std::vector<MeshVisualInfo *>> NodeAttachments;

	/** Indoor* */
	bool IndoorVob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** Current world transform */
	D3DXMATRIX WorldMatrix;

	/** BSP-Node this is stored in */
	std::vector<BspInfo*> ParentBSPNodes;
};

struct SectionInstanceCache
{
	SectionInstanceCache()
	{

	}

	~SectionInstanceCache();

	/** Clears the cache for the given progmesh */
	void ClearCacheForStatic(MeshVisualInfo* pm);

	std::map<MeshVisualInfo *, std::vector<VS_ExConstantBuffer_PerInstance>> InstanceCacheData;
	std::map<MeshVisualInfo *, BaseVertexBuffer*> InstanceCache;

};

class BaseTexture;

/** Describes a world-section for the renderer */
struct WorldMeshSectionInfo
{
	WorldMeshSectionInfo()
	{
		BoundingBox.Min = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
		BoundingBox.Max = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		FullStaticMesh = NULL;
	}

	~WorldMeshSectionInfo()
	{
		for(std::map<MeshKey, WorldMeshInfo*>::iterator it = WorldMeshes.begin(); it != WorldMeshes.end(); it++)
		{
			delete (*it).second;
		}
		WorldMeshes.clear();

		for(std::map<MeshKey, MeshInfo*>::iterator it = SuppressedMeshes.begin(); it != SuppressedMeshes.end(); it++)
		{
			delete (*it).second;
		}
		SuppressedMeshes.clear();
		
		for(std::map<BaseTexture *, std::vector<MeshInfo*>>::iterator it = WorldMeshesByCustomTexture.begin(); it != WorldMeshesByCustomTexture.end(); it++)
		{
			delete (*it).first; // Meshes are stored in "WorldMeshes". Only delete the texture
		}
		WorldMeshesByCustomTexture.clear();

		for(std::list<VobInfo *>::iterator it = Vobs.begin(); it != Vobs.end(); it++)
		{
			delete (*it);
		}
		Vobs.clear();

		for(auto it = SectionPolygons.begin(); it != SectionPolygons.end(); it++)
		{
			delete (*it);
		}
		SectionPolygons.clear();

		

		delete FullStaticMesh;
	}

	/** Saves this sections mesh to a file */
	void SaveSectionMeshToFile(const std::string& name);

	/** Saves the mesh infos for this section */
	void SaveMeshInfos(const std::string& worldName, INT2 sectionPos);

	/** Saves the mesh infos for this section */
	void LoadMeshInfos(const std::string& worldName, INT2 sectionPos);

	std::map<MeshKey, WorldMeshInfo*, cmpMeshKey> WorldMeshes;
	std::map<BaseTexture *, std::vector<MeshInfo*>> WorldMeshesByCustomTexture;
	std::map<zCMaterial *, std::vector<MeshInfo*>> WorldMeshesByCustomTextureOriginal;
	std::map<MeshKey, MeshInfo*, cmpMeshKey> SuppressedMeshes;
	std::list<VobInfo*> Vobs;

	/** Loaded ocean-polys of this section */
	std::vector<D3DXVECTOR3> OceanPoints;

	// This is filled in case we have loaded a custom worldmesh
	std::vector<zCPolygon *> SectionPolygons;

	/** The whole section as one single mesh, without alpha-test materials */
	MeshInfo* FullStaticMesh;

	/** This sections bounding box */
	zTBBox3D BoundingBox;

	/** XY-Coord on the section array */
	INT2 WorldCoordinates;

	SectionInstanceCache InstanceCache;

	unsigned int BaseIndexLocation;
	unsigned int NumIndices;
};

class zCBspTree;
class zCWorld;
struct WorldInfo
{
	WorldInfo()
	{
		BspTree = NULL;
		CustomWorldLoaded = false;
	}

	D3DXVECTOR2 MidPoint;
	float LowestVertex;
	float HighestVertex;
	zCBspTree* BspTree;
	zCWorld* MainWorld;
	std::string WorldName;
	bool CustomWorldLoaded;
};
