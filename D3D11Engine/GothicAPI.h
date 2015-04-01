#pragma once
#include "pch.h"
#include "GothicGraphicsState.h"
#include "WorldConverter.h"
#include "zCTree.h"
#include "zTypes.h"
#include "zCMorphMesh.h"

#define START_TIMING Engine::GAPI->GetRendererState()->RendererInfo.Timing.Start
#define STOP_TIMING Engine::GAPI->GetRendererState()->RendererInfo.Timing.Stop

const std::string MENU_SETTINGS_FILE = "system\\GD3D11\\UserSettings.bin";

class zCModelPrototype;
struct BspVobInfo
{
	BspVobInfo()
	{
		NumStaticLights = 0;
	}

	bool IsEmpty()
	{
		return Vobs.empty() && IndoorVobs.empty() && SmallVobs.empty() && Lights.empty() && IndoorLights.empty();
	}

	std::vector<VobInfo *> Vobs;
	std::vector<VobInfo *> IndoorVobs;
	std::vector<VobInfo *> SmallVobs;
	std::vector<VobLightInfo *> Lights;
	std::vector<VobLightInfo *> IndoorLights;
	int NumStaticLights;
};

struct CameraReplacement
{
	D3DXMATRIX ViewReplacement;
	D3DXMATRIX ProjectionReplacement;
	D3DXVECTOR3 PositionReplacement;
	D3DXVECTOR3 LookAtReplacement;
};

/** Version of this struct */
const int MATERIALINFO_VERSION = 3;

struct MaterialInfo
{
	enum EMaterialType
	{
		MT_None,
		MT_Water,
		MT_Ocean
	};


	MaterialInfo()
	{
		buffer.SpecularIntensity = 0.1f;
		buffer.SpecularPower = 60.0f;
		buffer.NormalmapStrength = 1.0f;
		buffer.DisplacementFactor = 1.0f;
		buffer.TextureScale = 1.0f;
		buffer.FresnelFactor = 0.5f;

		Constantbuffer = NULL;

		MaterialType = MT_None;

		VertexShader = "";
		PixelShader = "";
	}

	~MaterialInfo()
	{
		delete Constantbuffer;
	}

	/** Writes this info to a file */
	void WriteToFile(const std::string& name);

	/** Loads this info from a file */
	void LoadFromFile(const std::string& name);

	struct Buffer
	{
		float SpecularIntensity;
		float SpecularPower;
		float NormalmapStrength;
		float DisplacementFactor;

		float TextureScale;
		float FresnelFactor;
		float2 MI_Pad;
	};

	/** creates/updates the constantbuffer */
	void UpdateConstantbuffer();

	BaseConstantBuffer* Constantbuffer;

	std::string VertexShader;
	std::string TesselationShaderPair;
	std::string PixelShader;
	EMaterialType MaterialType;
	Buffer buffer;
};

struct ParticleFrameData
{
	unsigned char* Buffer;
	unsigned int BufferPosition;
	unsigned int BufferSize;
	unsigned int NeededSize;
};

/** Class used to communicate between Gothic and the Engine */
class zCPolygon;
class zCTexture;
class zCParticleFX;
class zCVisual;
class GSky;
class GMesh;
class zCBspBase;
class GInventory;
class zCVobLight;
class MyDirectDrawSurface7;
class GVegetationBox;
class GRenderThread;
class GOcean;
class zCDecal;
class GothicAPI
{
public:
	GothicAPI(void);
	~GothicAPI(void);

	/** Called when the game starts */
	void OnGameStart();

	/** Called when the window got set */
	void OnSetWindow(HWND hWnd);

	/** Message-Callback for the main window */
	LRESULT OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** Called when the game is about to load a new level */
	void OnLoadWorld(const std::string& levelName, int loadMode);

	/** Called when the game loaded a new level */
	void OnGeometryLoaded(zCPolygon** polys, unsigned int numPolygons);

	/** Called when the game is done loading the world */
	void OnWorldLoaded();

	/** Called to update the world, before rendering */
	void OnWorldUpdate();

	/** Called when a VOB got added to the BSP-Tree or the world */
	void OnAddVob(zCVob* vob, zCWorld* world);

	/** Called when a VOB got removed from the world */
	void OnRemovedVob(zCVob* vob, zCWorld* world);

	/** Called on a SetVisual-Call of a vob */
	void OnSetVisual(zCVob* vob);

	/** Called when a material got removed */
	void OnMaterialDeleted(zCMaterial* mat);

	/** Called when a particle system got removed */
	void OnParticleFXDeleted(zCParticleFX* pfx);

	/** Called when a visual got removed */
	void OnVisualDeleted(zCVisual* visual);
	

	/** Called when a material got removed */
	void OnMaterialCreated(zCMaterial* mat);

	/** Loads resources created for this .ZEN */
	void LoadCustomZENResources();

	/** Saves resources created for this .ZEN */
	void SaveCustomZENResources();

	/** Returns the GraphicsState */
	GothicRendererState* GetRendererState();

	/** Returns in which directory we started in */
	const std::string& GetStartDirectory();

	/** Draws the world-mesh */
	void DrawWorldMeshNaive();

	/** Draws a skeletal mesh-vob */
	void DrawSkeletalMeshVob(SkeletalVobInfo* vi, float distance);

	/** Draws the inventory */
	void DrawInventory(zCWorld* world, zCCamera& camera);

	/** Draws a morphmesh */
	void DrawMorphMesh(zCMorphMesh* msh, float fatness);


	/** Locks the resource CriticalSection */
	void EnterResourceCriticalSection();

	/** Unlocks the resource CriticalSection */
	void LeaveResourceCriticalSection();

	/** Draws a MeshInfo */
	void DrawMeshInfo(zCMaterial* mat, MeshInfo* msh);

	/** Draws a SkeletalMeshInfo */
	void DrawSkeletalMeshInfo(zCMaterial* mat, SkeletalMeshInfo* msh, SkeletalMeshVisualInfo* vis, std::vector<D3DXMATRIX>& transforms, float fatness = 1.0f);

	/** Draws a zCParticleFX */
	void DrawParticleFX(zCVob* source, zCParticleFX* fx, ParticleFrameData& data);

	/** Gets a list of visible decals */
	void GetVisibleDecalList(std::vector<zCVob *>& decals);

	/** Returns a list of visible particle-effects */
	void GetVisibleParticleEffectsList(std::vector<zCVob*>& pfxList);

	/** Sets the world matrix */
	void SetWorldTransform(const D3DXMATRIX& world);

	/** Sets the View matrix */
	void SetViewTransform(const D3DXMATRIX& view);

	/** Sets the Projection matrix */
	void SetProjTransform(const D3DXMATRIX& proj);

	/** Sets the Projection matrix */
	D3DXMATRIX GetProjTransform();

	/** Sets the world matrix */
	void SetWorldTransform(const D3DXMATRIX& world, bool transpose);

	/** Sets the world matrix */
	void SetViewTransform(const D3DXMATRIX& view, bool transpose);

	/** Sets the world matrix */
	void SetWorldViewTransform(const D3DXMATRIX& world, const D3DXMATRIX& view);

	/** Sets the world matrix */
	void ResetWorldTransform();

	/** Sets the world matrix */
	void ResetViewTransform();

	/** Debugging */
	void DrawTriangle();

	/** Removes the given quadmark */
	void RemoveQuadMark(zCQuadMark* mark);

	/** Saves all sections information */
	void SaveSectionInfos();

	/** Loads all sections information */
	void LoadSectionInfos();

	/** Returns wether the camera is underwater or not */
	bool IsUnderWater();

	/** Returns the quadmark info for the given mark. Creates a new one if needed. */
	QuadMarkInfo* GetQuadMarkInfo(zCQuadMark* mark);

	/** Returns all quad marks */
	const stdext::hash_map<zCQuadMark*, QuadMarkInfo>& GetQuadMarks();

	/** Returns the loaded sections */
	std::map<int, std::map<int, WorldMeshSectionInfo>>& GetWorldSections();

	/** Returns the wrapped world mesh */
	MeshInfo* GetWrappedWorldMesh();

	/** Returns the loaded skeletal mesh vobs */
	std::list<SkeletalVobInfo *>& GetSkeletalMeshVobs();

	/** Returns the current cameraposition */
	D3DXVECTOR3 GetCameraPosition();

	/** Returns the current forward vector of the camera */
	D3DXVECTOR3 GetCameraForward();

	/** Returns the view matrix */
	void GetViewMatrix(D3DXMATRIX* view);

	/** Returns the view matrix */
	void GetInverseViewMatrix(D3DXMATRIX* invView);

	/** Returns the projection-matrix */
	D3DXMATRIX& GetProjectionMatrix();

	/** Unprojects a pixel-position on the screen */
	void Unproject(const D3DXVECTOR3& p, D3DXVECTOR3* worldPos, D3DXVECTOR3* worldDir);

	/** Unprojects the current cursor, returns it's direction in world-space */
	D3DXVECTOR3 UnprojectCursor();

	/** Traces the worldmesh and returns the hit-location */
	bool TraceWorldMesh(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, D3DXVECTOR3& hit, std::string* hitTextureName = NULL, D3DXVECTOR3* hitTriangle = NULL, MeshInfo** hitMesh = NULL, zCMaterial** hitMaterial = NULL);

	/** Traces vobs with static mesh visual */
	VobInfo* TraceStaticMeshVobsBB(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, D3DXVECTOR3& hit, zCMaterial** hitMaterial = NULL);
	SkeletalVobInfo* TraceSkeletalMeshVobsBB(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, D3DXVECTOR3& hit);

	/** Traces a visual info. Returns -1 if not hit, distance otherwise */
	float TraceVisualInfo(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, BaseVisualInfo* visual, zCMaterial** hitMaterial = NULL);



	/** Returns the GSky-Object */
	GSky* GetSky();

	/** Returns the fog-color */
	D3DXVECTOR3 GetFogColor();

	/** Returns true if the game is overwriting the fog color with a fog-zone */
	float GetFogOverride();

	/** Returns the inventory */
	GInventory* GetInventory();

	/** Returns if the material is currently active */
	bool IsMaterialActive(zCMaterial* mat);

	/** Sets the current input state. Keeps an internal count of how many times it was disabled. */
	void SetEnableGothicInput(bool value);

	/** Returns the midpoint of the current world */
	WorldInfo* GetLoadedWorldInfo(){return LoadedWorldInfo;}

	/** Returns wether the camera is indoor or not */
	bool IsCameraIndoor();

	/** Returns gothics fps-counter */
	int GetFramesPerSecond();

	/** Returns the current frame time */
	float GetFrameTimeSec();

	/** Returns global time */
	float GetTimeSeconds();

	/** Builds the static mesh instancing cache */
	void BuildStaticMeshInstancingCache();

	/** Draws the AABB for the BSP-Tree using the line renderer*/
	void DebugDrawBSPTree();

	/** Recursive helper function to draw the BSP-Tree */
	void DebugDrawTreeNode(zCBspBase* base, zTBBox3D boxCell, int clipFlags = 63);

	/** Draws particles, in a simple way */
	void DrawParticlesSimple();

	/** Moves the given vob from a BSP-Node to the dynamic vob list */
	void MoveVobFromBspToDynamic(VobInfo* vob);
	std::vector<VobInfo *>::iterator MoveVobFromBspToDynamic(VobInfo* vob, std::vector<VobInfo *>* source);

	/** Collects vobs using gothics BSP-Tree */
	void CollectVisibleVobs(std::vector<VobInfo *>& vobs, std::vector<VobLightInfo *>& lights);

	/** Collects visible sections from the current camera perspective */
	void CollectVisibleSections(std::list<WorldMeshSectionInfo*>& sections);

	/** Builds our BspTreeVobMap */
	void BuildBspVobMapCache();

	/** Disables a problematic method which causes the game to conflict with other applications on startup */
	static void DisableErrorMessageBroadcast();

	/** Sets/Gets the far-plane */
	void SetFarPlane(float value);
	float GetFarPlane();

	/** Sets/Gets the far-plane */
	void SetNearPlane(float value);
	float GetNearPlane();

	/** Returns true if the given string can be found in the commandline */
	bool HasCommandlineParameter(const std::string& param);

	/** Gets the int-param from the ini. String must be UPPERCASE. */
	int GetIntParamFromConfig(const std::string& param);

	/** Sets the given int param into the internal ini-cache. That does not set the actual value for the game! */
	void SetIntParamFromConfig(const std::string& param, int value);

	/** Resets the object, like at level load */
	void ResetWorld();

	/** Resets only the vobs */
	void ResetVobs();

	/** Get material by texture name */
	zCMaterial* GetMaterialByTextureName(const std::string& name);
	void GetMaterialListByTextureName(const std::string& name, std::list<zCMaterial*>& list);

	/** Returns the time since the last frame */
	float GetDeltaTime();

	/** If this returns true, the property holds the name of the currently bound texture. If that is the case, any MyDirectDrawSurfaces should not bind themselfes
		to the pipeline, but rather check if there are additional textures to load */
	bool IsInTextureTestBindMode(std::string& currentBoundTexture);

	/** Sets the current texture test bind mode status */
	void SetTextureTestBindMode(bool enable, const std::string& currentTexture);

	/** Sets the CameraReplacementPtr */
	void SetCameraReplacementPtr(CameraReplacement* ptr){CameraReplacementPtr = ptr;}

	/** Lets Gothic draw its sky */
	void DrawSkyGothicOriginal();

	/** Returns the material info associated with the given material */
	MaterialInfo* GetMaterialInfoFrom(zCTexture* tex);

	/** Adds a surface */
	void AddSurface(const std::string& name, MyDirectDrawSurface7* surface);

	/** Gets a surface by texturename */
	MyDirectDrawSurface7* GetSurface(const std::string& name);

	/** Removes a surface */
	void RemoveSurface(MyDirectDrawSurface7* surface);

	/** Resets all vob-stats drawn this frame */
	void ResetVobFrameStats(std::list<VobInfo *>& vobs);

	/** Sets the currently bound texture */
	void SetBoundTexture(int idx, zCTexture* tex);
	zCTexture* GetBoundTexture(int idx);

	/** Returns gothics output window */
	HWND GetOutputWindow(){return OutputWindow;}

	/** Spawns a vegetationbox at the camera */
	GVegetationBox* SpawnVegetationBoxAt(const D3DXVECTOR3& position, 
		const D3DXVECTOR3& min = D3DXVECTOR3(-1000, -500, -1000), 
		const D3DXVECTOR3& max = D3DXVECTOR3(1000, 500, 1000), 
		float density = 1.0f,
		const std::string& restrictByTexture = "");

	/** Adds a vegetationbox to the world */
	void AddVegetationBox(GVegetationBox* box);

	/** Returns the list of current GVegentationBoxes */
	const std::list<GVegetationBox*>& GetVegetationBoxes(){return VegetationBoxes;}

	/** Removes a vegetationbox from the world */
	void RemoveVegetationBox(GVegetationBox* box);

	/** Teleports the player to the given location */
	void SetPlayerPosition(const D3DXVECTOR3& pos);

	/** Returns the map of static mesh visuals */
	const std::hash_map<zCProgMeshProto*, MeshVisualInfo*>& GetStaticMeshVisuals(){return StaticMeshVisuals;}

	/** Removes the given texture from the given section and stores the supression, so we can load it next time */
	void SupressTexture(WorldMeshSectionInfo* section, const std::string& texture);

	/** Resets the suppressed textures */
	void ResetSupressedTextures();

	/** Resets the vegetation */
	void ResetVegetation();

	/** Saves Suppressed textures to a file */
	XRESULT SaveSuppressedTextures(const std::string& file);

	/** Saves Suppressed textures to a file */
	XRESULT LoadSuppressedTextures(const std::string& file);

	/** Saves vegetation to a file */
	XRESULT SaveVegetation(const std::string& file);

	/** Saves vegetation to a file */
	XRESULT LoadVegetation(const std::string& file);

	/** Sets/Gets the pending movie frame */
	void SetPendingMovieFrame(BaseTexture* frame);
	BaseTexture* GetPendingMovieFrame();

	/** Returns the main-thread id */
	DWORD GetMainThreadID();

	/** Returns the current cursor position, in pixels */
	POINT GetCursorPosition();

	/** Saves the users settings from the menu */
	XRESULT SaveMenuSettings(const std::string& file);

	/** Loads the users settings from the menu */
	XRESULT LoadMenuSettings(const std::string& file);

	/** Clears the array of this frame loaded textures */
	void ClearFrameLoadedTextures();

	/** Adds a texture to the list of the loaded textures for this frame */
	void AddFrameLoadedTexture(MyDirectDrawSurface7* srf);

	/** Sets loaded textures of this frame ready */
	void SetFrameLoadedTexturesReady();

	/** Returns if the given vob is registered in the world */
	SkeletalVobInfo* GetSkeletalVobByVob(zCVob* vob);

	/** Returns the frame particle info collected from all DrawParticleFX-Calls */
	std::map<zCTexture*, ParticleRenderInfo>& GetFrameParticleInfo();
private:
	/** Cleans empty BSPNodes */
	void CleanBSPNodes();

	/** Helper function for going through the bsp-tree */
	void BuildBspVobMapCacheHelper(zCBspBase* base);

	/** Recursive helper function to draw collect the vobs */
	void CollectVisibleVobsHelper(zCBspBase* base, zTBBox3D boxCell, int clipFlags, std::vector<VobInfo *>& vobs, std::vector<VobLightInfo  *>& lights);

	/** Applys the suppressed textures */
	void ApplySuppressedSectionTextures();


	/** Hooked Window-Proc from the game */
	static LRESULT CALLBACK GothicWndProc(
		HWND hWnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
		);


	/** Debugging */
	void DrawTestInstances();

	/** Goes through the given zCTree and registeres all found vobs */
	void TraverseVobTree(zCTree<zCVob>* tree);

	/** Saved Graphics state */
	GothicRendererState RendererState;

	/** Loaded world mitpoint */
	WorldInfo* LoadedWorldInfo;

	/** Currently bound textures from gothic */
	zCTexture* BoundTextures[8];

	std::map<zCTexture*, std::vector<ParticleInstanceInfo>> FrameParticles;
	std::map<zCTexture*, ParticleRenderInfo> FrameParticleInfo;

	/** Loaded game sections */
	std::map<int, std::map<int, WorldMeshSectionInfo>> WorldSections;
	MeshInfo* WrappedWorldMesh;

	/** List of vobs with skeletal meshes (Having a zCModel-Visual) */
	std::list<SkeletalVobInfo *> SkeletalMeshVobs;

	/** List of Vobs having a zCParticleFX-Visual */
	std::list<zCVob *> ParticleEffectVobs;
	std::list<zCVob *> DecalVobs;
	std::hash_map<zCVob *, std::string> tempParticleNames;

	/** Set of Materials */
	std::set<zCMaterial *> LoadedMaterials;

	/** List of meshes rendered for this frame */
	std::set<MeshVisualInfo *> FrameMeshInstances;

	/** Map for static mesh visuals */
	std::hash_map<zCProgMeshProto*, MeshVisualInfo*> StaticMeshVisuals;

	/** Map for skeletal mesh visuals */
	std::hash_map<std::string, SkeletalMeshVisualInfo*> SkeletalMeshVisuals;

	/** Set of all vobs we registered by now */
	std::set<zCVob*> RegisteredVobs;

	/** List of dynamically added vobs */
	std::list<VobInfo*> DynamicallyAddedVobs;

	/** Map of vobs and VobIndfos */
	std::hash_map<zCVob*, VobInfo*> VobMap;
	std::hash_map<zCVobLight*, VobLightInfo*> VobLightMap;
	std::hash_map<zCVob*, SkeletalVobInfo*> SkeletalVobMap;

	/** Map of VobInfo-Lists for zCBspLeafs */
	std::hash_map<zCBspBase *, BspVobInfo> BspLeafVobLists;

	/** Map for the material infos */
	std::hash_map<zCTexture*, MaterialInfo> MaterialInfos;

	/** Maps visuals to vobs */
	std::hash_map<zCVisual*, std::list<BaseVobInfo *>> VobsByVisual;

	/** Map of textures */
	std::hash_map<std::string, MyDirectDrawSurface7*> SurfacesByName;

	/** Directory we started in */
	std::string StartDirectory;

	/** Resource critical section */
	CRITICAL_SECTION ResourceCriticalSection;
	std::mutex ResourceMutex;

	/** Sky renderer */
	GSky* SkyRenderer;

	/** Inventory manager */
	GInventory* Inventory;

	/** Saved Wnd-Proc pointer from the game */
	LONG_PTR OriginalGothicWndProc;

	/** Whether we test texture binds to figure out what surface uses which zCTexture object */
	bool TextureTestBindMode;
	std::string BoundTestTexture;

	/** Replacement values for the camera */
	CameraReplacement* CameraReplacementPtr;

	/** List of available GVegetationBoxes */
	std::list<GVegetationBox*> VegetationBoxes;

	/** Gothics output window */
	HWND OutputWindow;

	/** Renderthread */
	GRenderThread* RenderThread;

	/** Ocean */
	GOcean* Ocean;

	/** Suppressed textures for the sections */
	std::map<WorldMeshSectionInfo*, std::vector<std::string>> SuppressedTexturesBySection;

	/** Current camera, stored to find out about camera switches */
	zCCamera* CurrentCamera;

	/** Currently pending movie frame */
	BaseTexture* PendingMovieFrame;

	/** The id of the main thread */
	DWORD MainThreadID;

	/** Textures loaded this frame */
	std::vector<MyDirectDrawSurface7 *> FrameLoadedTextures;

	/** Quad marks loaded in the world */
	stdext::hash_map<zCQuadMark*, QuadMarkInfo> QuadMarks;

	/** Map of parameters from the .ini */
	std::map<std::string, int> ConfigIntValues;
};

