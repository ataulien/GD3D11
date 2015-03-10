#pragma once
#include "pch.h"
#include "BaseVertexBuffer.h"
#include "BaseConstantBuffer.h"
#include "ConstantBufferStructs.h"
#include "zTypes.h"
#include "BaseTexture.h"
#include "GothicGraphicsState.h"


/** Square size of a single world-section */
const float WORLD_SECTION_SIZE = 16000;

const float4 DEFAULT_LIGHTMAP_POLY_COLOR_F = float4(0.20f, 0.20f,0.20f,0.20f);
const DWORD DEFAULT_LIGHTMAP_POLY_COLOR = DEFAULT_LIGHTMAP_POLY_COLOR_F.ToDWORD();
const float3 DEFAULT_INDOOR_VOB_AMBIENT = float3(0.25f, 0.25f, 0.25f);

class zCMaterial;
class zCPolygon;
class BaseVertexBuffer;
class zCVob;
class zCTexture;
class zCLightmap;
struct zCModelNodeInst;
struct BspVobInfo;
class zCQuadMark;
struct MaterialInfo;

struct ParticleRenderInfo
{
	GothicBlendStateInfo BlendState;
	int BlendMode;
};

struct ParticleInstanceInfo
{
	D3DXMATRIX worldview;
	DWORD color;
	float2 scale;
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

		UpdateConstantbuffer();
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
		float VT_Pad1;
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

	}

	~MeshInfo();

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

	~BaseVisualInfo()
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

	}

	/** Starts a new frame for this mesh */
	void StartNewFrame()
	{
		Instances.clear();
	}

	/** Creates PNAEN-Info for all meshes if not already there */
	void CreatePNAENInfo(bool softNormals = false);

	std::map<MeshKey, std::vector<MeshInfo*>, cmpMeshKey> MeshesByTexture;
	
	zCProgMeshProto* Visual;
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

	/** Map of visuals attached to nodes */
	std::map<int, std::vector<MeshVisualInfo *>> NodeAttachments;

	/** Submeshes of this visual */
	std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>> SkeletalMeshes;
};

struct BaseVobInfo
{
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
	std::vector<BspVobInfo*> ParentBSPNodes;
};

class zCVobLight;
struct VobLightInfo
{
	VobLightInfo()
	{
		Vob = NULL;
		VisibleInRenderPass = false;
		IsIndoorVob = false;
	}

	/** Vob the data came from */
	zCVobLight* Vob;

	/** Flag to see if this vob was drawn in the current render pass. Used to collect the same vob only once. */
	bool VisibleInRenderPass;

	/** True if this is an indoor-vob */
	bool IsIndoorVob;

	/** BSP-Node this is stored in */
	std::vector<BspVobInfo*> ParentBSPNodes;
};


/** Holds the converted mesh of a VOB */
struct SkeletalVobInfo : public BaseVobInfo
{
	SkeletalVobInfo()
	{
		Vob = NULL;
		VisualInfo = NULL;
		IndoorVob = false;
	}

	~SkeletalVobInfo()
	{
		//delete VisualInfo;
	}

	/** Indoor* */
	bool IndoorVob;
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
		for(std::map<MeshKey, MeshInfo*>::iterator it = WorldMeshes.begin(); it != WorldMeshes.end(); it++)
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

		delete FullStaticMesh;
	}

	/** Saves this sections mesh to a file */
	void SaveSectionMeshToFile(const std::string& name);

	std::map<MeshKey, MeshInfo*, cmpMeshKey> WorldMeshes;
	std::map<BaseTexture *, std::vector<MeshInfo*>> WorldMeshesByCustomTexture;
	std::map<zCMaterial *, std::vector<MeshInfo*>> WorldMeshesByCustomTextureOriginal;
	std::map<MeshKey, MeshInfo*, cmpMeshKey> SuppressedMeshes;
	std::list<VobInfo*> Vobs;

	/** Loaded ocean-polys of this section */
	std::vector<D3DXVECTOR3> OceanPoints;

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
	}

	D3DXVECTOR2 MidPoint;
	float LowestVertex;
	float HighestVertex;
	zCBspTree* BspTree;
	zCWorld* MainWorld;
	std::string WorldName;
};

class zCProgMeshProto;
class zCModel;
class zCMesh;
class WorldConverter
{
public:
	WorldConverter(void);
	virtual ~WorldConverter(void);

	/** Converts the worldmesh into a more usable format */
	static HRESULT ConvertWorldMesh(zCPolygon** polys, unsigned int numPolygons, std::map<int, std::map<int, WorldMeshSectionInfo>>* outSections, WorldInfo* info, MeshInfo** outWrappedMesh);

	/** Converts the worldmesh into a PNAEN-buffer */
	static HRESULT ConvertWorldMeshPNAEN(zCPolygon** polys, unsigned int numPolygons, std::map<int, std::map<int, WorldMeshSectionInfo>>* outSections, WorldInfo* info, MeshInfo** outWrappedMesh);

	/** Converts a loaded custommesh to be the worldmesh */
	static XRESULT LoadWorldMeshFromFile(const std::string& file, std::map<int, std::map<int, WorldMeshSectionInfo>>* outSections, WorldInfo* info, MeshInfo** outWrappedMesh);

	/** Returns what section the given position is in */
	static INT2 GetSectionOfPos(const D3DXVECTOR3& pos);

	/** Converts a world polygon triangle fan to a vertex list */
	static void TriangleFanToList(ExVertexStruct* input, unsigned int numInputVertices, std::vector<ExVertexStruct>* outVertices);

	/** Saves the given section-array to an obj file */
	static void SaveSectionsToObjUnindexed(const char* file, const std::map<int, std::map<int, WorldMeshSectionInfo>>& sections);

	/** Saves the given prog mesh to an obj-file */
	//static void SaveProgMeshToOBj(

	/** Extracts a 3DS-Mesh from a zCVisual */
	static void Extract3DSMeshFromVisual(zCProgMeshProto* visual, MeshVisualInfo* meshInfo);

	/** Extracts a 3DS-Mesh from a zCVisual */
	static void Extract3DSMeshFromVisual2(zCProgMeshProto* visual, MeshVisualInfo* meshInfo);
	static void Extract3DSMeshFromVisual2PNAEN(zCProgMeshProto* visual, MeshVisualInfo* meshInfo);

	/** Extracts a skeletal mesh from a zCModel */
	static void ExtractSkeletalMeshFromVob(zCModel* model, SkeletalMeshVisualInfo* skeletalMeshInfo);
	static void ExtractSkeletalMeshFromVobPNAEN(zCModel* model, SkeletalMeshVisualInfo* skeletalMeshInfo);
	
	/** Extracts a node-visual */
	static void ExtractNodeVisual(int index, zCModelNodeInst* node, SkeletalMeshVisualInfo* skeletalMeshInfo);

	/** Updates a quadmark info */
	static void UpdateQuadMarkInfo(QuadMarkInfo* info, zCQuadMark* mark, const D3DXVECTOR3& position);

	/** Indexes the given vertex array */
	static void IndexVertices(ExVertexStruct* input, unsigned int numInputVertices, std::vector<ExVertexStruct>& outVertices, std::vector<VERTEX_INDEX>& outIndices);
	static void IndexVertices(ExSkelVertexStruct* input, unsigned int numInputVertices, std::vector<ExSkelVertexStruct>& outVertices, std::vector<VERTEX_INDEX>& outIndices);
	static void IndexVertices(ExVertexStruct* input, unsigned int numInputVertices, std::vector<ExVertexStruct>& outVertices, std::vector<unsigned int>& outIndices);

	/** Marks the edges of the mesh */
	static void MarkEdges(std::vector<ExVertexStruct>& vertices, std::vector<VERTEX_INDEX>& indices);

	/** Computes vertex normals for a mesh with face normals */
	static void GenerateVertexNormals(std::vector<ExVertexStruct>& vertices, std::vector<VERTEX_INDEX>& indices);

	/** Creates the FullSectionMesh for the given section */
	static void GenerateFullSectionMesh(WorldMeshSectionInfo& section);

	/** Tesselates the given triangle and adds the values to the list */
	static void TesselateTriangle(ExVertexStruct* tri, std::vector<ExVertexStruct>& tesselated, int amount);

	/** Builds a big vertexbuffer from the world sections */
	static void WrapVertexBuffers(const std::list<std::vector<ExVertexStruct>*>& vertexBuffers, const std::list<std::vector<VERTEX_INDEX>*>& indexBuffers, std::vector<ExVertexStruct>& outVertices, std::vector<unsigned int>& outIndices, std::vector<unsigned int>& outOffsets);

	/** Caches a mesh */
	static void CacheMesh(const std::map<std::string, std::vector<std::pair<std::vector<ExVertexStruct>, std::vector<VERTEX_INDEX>>>> geometry, const std::string& file);

	/** Turns a MeshInfo into PNAEN */
	static void CreatePNAENInfoFor(MeshInfo* mesh, bool softNormals = false);
	static void CreatePNAENInfoFor(SkeletalMeshInfo* mesh, MeshInfo* bindPoseMesh, bool softNormals = false);
};

