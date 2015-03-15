#pragma once

/** Holds all memorylocations for the selected game */
struct GothicMemoryLocations
{
	struct Functions
	{
		static const unsigned int Alg_Rotation3DNRad = 0x00514D10;
		static const int vidGetFPSRate = 0x004FB7B0;
		static const unsigned int HandledWinMain = 0x00500160;
	};

	struct GlobalObjects
	{
		static const unsigned int zCResourceManager = 0x008F5950;
		static const unsigned int zCCamera = 0x0089A0FC;
		static const unsigned int oCGame = 0x0090168C;
		static const unsigned int zCTimer = 0x008F6170;
		static const unsigned int DInput7DeviceMouse = 0x00894058;
		static const unsigned int DInput7DeviceKeyboard = 0x0089404C;
		static const unsigned int zCInput = 0x00893A00;
		static const unsigned int s_globFreePart = 0x0089B2B4;
		static const unsigned int zCOption = 0x008902FC;
	};

	struct zCQuadMark
	{
		static const unsigned int Constructor = 0x005BCCF0;
		static const unsigned int Destructor = 0x005BCF40;
		static const unsigned int CreateQuadMark = 0x005BE410;
		static const unsigned int Offset_QuadMesh = 0x34;
		static const unsigned int Offset_Material = 0x3C;
		static const unsigned int Offset_ConnectedVob = 0x38;
	};

	struct zCRndD3D
	{
		static const unsigned int VidSetScreenMode = 0x00658BA0;
		static const unsigned int DrawLineZ = 0x00738C80;
	};

	struct zCOption
	{
		static const unsigned int ReadInt = 0x00463120;
		static const unsigned int ReadBool = 0x00462EA0;
		static const unsigned int Offset_CommandLine = 0x284;
	};

	struct zFILE
	{
		static const unsigned int Open = 0x00443410;
	};

	struct zCMesh
	{
		static const unsigned int Offset_Polygons = 0x44;
		static const unsigned int Offset_PolyArray = 0x50;
		static const unsigned int Offset_NumPolygons = 0x34;
		static const unsigned int CreateListsFromArrays = 0x005671C0;
		static const unsigned int CreateTriMesh = 0x00563270;
	};

	struct zCMeshSoftSkin
	{
		//static const unsigned int Offset_NodeIndexList = 0x38C;
		static const unsigned int Offset_VertWeightStream = 0x0F8;
	};

	struct zCSkyController_Outdoor
	{
		static const unsigned int OBJ_ActivezCSkyController = 0x0099AC8C;

		//static const unsigned int Offset_Sun = 0x5F4; // First of the two planets
		static const unsigned int Offset_MasterTime = 0x6C;
		static const unsigned int Offset_MasterState = 0x74;
		/*static const unsigned int Offset_SkyLayer1 = 0x5A8;
		static const unsigned int Offset_SkyLayer2 = 0x5C4;
		static const unsigned int Offset_SkyLayerState = 0x5A0;		
		static const unsigned int Offset_SkyLayerState0 = 0x120;
		static const unsigned int Offset_SkyLayerState1 = 0x124;
		static const unsigned int Interpolate = 0x005E8C20;
		static const unsigned int Offset_InitDone = 0x7C;
		static const unsigned int Init = 0x005E6A00;*/
	};

	struct zCParticleEmitter
	{
		static const unsigned int Offset_VisTexture = 0x2DC;
		static const unsigned int Offset_VisAlphaBlendFunc = 0x308;
	};

	struct zCStaticPfxList
	{
		static const unsigned int TouchPFX = 0x0059E5D0;
	};

	struct zERROR
	{
		// Start/End for problematic SendMessage-Broadcast which causes the game to conflict with other applications
		static const unsigned int BroadcastStart = 0x0044CB2D;
		static const unsigned int BroadcastEnd = 0x0044CB3C;
	};

	struct zCCamera
	{
		static const unsigned int GetTransform = 0x00544700;
		static const unsigned int SetTransform = 0x005445A0;
		static const unsigned int UpdateViewport = 0x00544AF0;
		static const unsigned int Activate = 0x00544760;
		static const unsigned int BBox3DInFrustum = 0x00545110;
		
	};

	struct zCVob
	{
		static const unsigned int Offset_WorldMatrixPtr = 0x3C;
		//static const unsigned int Offset_BoundingBoxWS = 0x40;
		static const unsigned int GetVisual = 0x005FCBE0;
		static const unsigned int SetVisual = 0x005E95E0;
		static const unsigned int GetPositionWorld = 0x00528FC0;
		static const unsigned int Offset_GroundPoly = 0x0AC;
		static const unsigned int DoFrameActivity = 0x005E9DE0;
		static const unsigned int GetBBoxLocal = 0x00600F30;
		static const unsigned int Offset_HomeWorld = 0x0A8;

		static const unsigned int Offset_Flags = 0x104;
		static const unsigned int MASK_ShowVisual = 0x1;
		static const unsigned int Offset_CameraAlignment = 0x110;
		static const unsigned int SHIFTLR_CameraAlignment = 0x1E;

		static const unsigned int Offset_WorldPosX = 0x48;
		static const unsigned int Offset_WorldPosY = 0x58;
		static const unsigned int Offset_WorldPosZ = 0x68;
	};

	struct zCInput
	{
		static const unsigned int GetDeviceEnabled = 0x004D2A00;
		static const unsigned int SetDeviceEnabled = 0x004D29C0;
	};

	struct zCVisual
	{
		static const unsigned int VTBL_GetFileExtension = 17;
		static const unsigned int Destructor = 0x005EC8F2;
	};

	struct zCBspTree
	{
		static const unsigned int AddVob = 0x0052C280;
		static const unsigned int LoadBIN = 0x00532FE0;
		static const unsigned int Offset_NumPolys = 0x24;
		static const unsigned int Offset_PolyArray = 0x10;
		static const unsigned int Offset_RootNode = 0x8;
		static const unsigned int Offset_WorldMesh = 0xC;
		static const unsigned int Offset_LeafList = 0x18;
		static const unsigned int Offset_NumLeafes = 0x20;
		static const unsigned int Offset_ArrPolygon = 0x4C;
		static const unsigned int Offset_BspTreeMode = 0x58;
	};

	struct zCPolygon
	{
		static const unsigned int Offset_VerticesArray = 0x00;
		static const unsigned int Offset_FeaturesArray = 0x2C;
		static const unsigned int Offset_NumPolyVertices = 0x30;
		static const unsigned int Offset_PolyFlags = 0x31;
		static const unsigned int Offset_Material = 0x18;
		static const unsigned int Offset_Lightmap = 0x1C;
		
		static const unsigned int Size = 0x38;
	};

	struct zCThread
	{
		static const unsigned int SuspendThread = 0x005E04E0;
		static const unsigned int Offset_SuspendCount = 0x0C;
	};

	struct zCBspLeaf
	{
		static const unsigned int Size = 0x5C;
	};

	struct zSTRING
	{
		static const unsigned int ToChar = 0x00464770;
	};

	struct zCModelPrototype
	{
		static const unsigned int Offset_NodeList = 0x74;
	};

	struct zCModelTexAniState
	{
		static const unsigned int UpdateTexList = 0x0056BDA0;
	};

	struct zCMaterial
	{
		static const unsigned int Offset_Texture = 0x34;
		static const unsigned int Offset_AlphaFunc = 0x74;
		static const unsigned int Offset_MatGroup = 0x40;
		static const unsigned int Offset_TexAniCtrl = 0x4C;

		static const unsigned int InitValues = 0x0055C000;
		static const unsigned int Constructor = 0x0055BBE0;
		static const unsigned int Destructor = 0x0055BE50;
		static const unsigned int GetAniTexture = 0x00737340;
		
	};

	struct zCObject
	{
		//static const unsigned int Release = 0x0040C310;
		static const unsigned int GetObjectName = 0x0059AF90;
	};

	struct zCTexture
	{
		static const unsigned int zCTex_D3DInsertTexture = 0x0073FE80;
		static const unsigned int LoadResourceData = 0x005DC6B0;
		static const unsigned int Offset_CacheState = 0x4C;
		static const unsigned int Mask_CacheState = 3;
		static const unsigned int GetName = 0x0059AF90;
		//static const unsigned int GetAniTexture = 0x005DA9B0;

		static const unsigned int Offset_Flags = 0x88;
		static const unsigned int Mask_FlagHasAlpha = 0x1;
		static const unsigned int Mask_FlagIsAnimated = 0x2;
		
		static const unsigned int zCResourceTouchTimeStamp = 0x005C6A30;
		static const unsigned int zCResourceTouchTimeStampLocal = 0x005C6AC0;

		static const unsigned int Offset_Surface = 0xD4;
	};

	struct zCBinkPlayer
	{
		static const unsigned int GetPixelFormat = 0x004406B0;
	};

	struct oCSpawnManager
	{
		static const unsigned int SpawnNpc = 0x006EDD80;

	};

	struct zCDecal
	{
		static const unsigned int Offset_DecalSettings = 0x34;
		static const unsigned int GetAlphaTestEnabled = 0x00556BE0;
	};

	struct zCResourceManager
	{
		static const unsigned int CacheIn = 0x005C7240;
	};

	struct zCProgMeshProto
	{
		static const unsigned int Offset_PositionList = 0x34;
		static const unsigned int Offset_NormalsList = 0x38;
		static const unsigned int Offset_Submeshes = 0xA4;
		static const unsigned int Offset_NumSubmeshes = 0xA8;
	};

	struct zCWorld
	{
		static const unsigned int Render = 0x006073A0;
		static const unsigned int LoadWorld = 0x0060C1B0;
		static const unsigned int VobRemovedFromWorld = 0x006098A0;
		static const unsigned int VobAddedToWorld = 0x006098A0;
		static const unsigned int Offset_GlobalVobTree = 0x24;
		static const unsigned int Call_Render_zCBspTreeRender = 0x00607475;
		static const unsigned int GetActiveSkyController = 0x006060A0;
		static const unsigned int Offset_SkyControllerOutdoor = 0x0D0;
		static const unsigned int DisposeVobs = 0x00608AD0;
	};

	struct oCWorld
	{
		static const unsigned int InsertVobInWorld = 0x006F4C60;
	};

	struct zCBspNode
	{
		static const unsigned int RenderIndoor = 0x0052A3E0;
		static const unsigned int RenderOutdoor = 0x0052A7B0;
		static const unsigned int REPL_RenderIndoorEnd = 0x0052A47D;
		
		static const unsigned int REPL_RenderOutdoorStart = 0x0052A7B0;
		static const unsigned int REPL_RenderOutdoorEnd = 0x0052A9BA;
		static const unsigned int SIZE_RenderOutdoor = 0x252;

	};

	struct oCNPC
	{
		static const unsigned int ResetPos = 0x007723E0;
		static const unsigned int Enable = 0x006BDAC0;
	};

	struct oCGame
	{
		static const unsigned int EnterWorld = 0x00655970;
		static const unsigned int TestKeys = 0x00678DF0;
		static const unsigned int Var_Player = 0x00902C10;
		
	};

	struct zCView
	{
		static const unsigned int SetMode = 0x00722A00;
		static const unsigned int REPL_SetMode_ModechangeStart = 0x00722A29;
		static const unsigned int REPL_SetMode_ModechangeEnd = 0x00722A38;
	};

	struct zCVobLight
	{
		static const unsigned int Offset_LightColor = 0x140; // Right before Range
		static const unsigned int Offset_Range = 0x144;
		static const unsigned int Offset_LightInfo =  0x164;
		static const unsigned int Mask_LightEnabled = 0x20;
	};

	struct zCMorphMesh
	{
		static const unsigned int Offset_MorphMesh = 0x38;
		static const unsigned int Offset_TexAniState = 0x40;
	};

	struct zCParticleFX
	{
		static const unsigned int Offset_FirstParticle = 0x34;
		static const unsigned int Offset_TimeScale = 0x8C;
		static const unsigned int Offset_LocalFrameTimeF = 0x90; // Offset_TimeScale + 4
		static const unsigned int Offset_PrivateTotalTime = 0x84; // Offset_TimeScale - 8
		static const unsigned int Offset_Emitters = 0x54;

		static const unsigned int OBJ_s_pfxList = 0x0089B2A0;
		static const unsigned int OBJ_s_partMeshQuad = 0x0089B2BC;
		static const unsigned int CheckDependentEmitter = 0x005A2350;
		static const unsigned int UpdateParticle = 0x005A0630;

		//static const unsigned int OBJ_s_pfxList = 0x008D9214;

		static const unsigned int CreateParticlesUpdateDependencies = 0x005A03B0;
		static const unsigned int SetVisualUsedBy = 0x0059EE30;
		static const unsigned int Destructor = 0x0059E370;
		static const unsigned int UpdateParticleFX = 0x005A02D0;
	};

	struct zCModel
	{
		static const unsigned int RenderNodeList = 0x0056E210;
		static const unsigned int UpdateAttachedVobs = 0x00575280;
		static const unsigned int Offset_ModelProtoList = 0x58;
		static const unsigned int Offset_NodeList = 0x64;
		static const unsigned int Offset_MeshSoftSkinList = 0x70;
		static const unsigned int Offset_MeshLibList = 0xB0;
		static const unsigned int Offset_AttachedVobList = 0x8C;
		static const unsigned int Offset_Flags = 0x1F8;
		//static const unsigned int Offset_DrawHandVisualsOnly = 0x174;
		static const unsigned int Offset_ModelFatness = 0x118;
		static const unsigned int Offset_ModelScale = 0x11C;
		static const unsigned int Offset_DistanceModelToCamera = 0x114;
		static const unsigned int GetVisualName = 0x005727F0;
	};
};