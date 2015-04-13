#pragma once

/** Holds all memorylocations for the selected game */
struct GothicMemoryLocations
{
	struct Functions
	{
		static const unsigned int Alg_Rotation3DNRad = 0x005081A0;
		static const int vidGetFPSRate = 0x004EF790;
		static const unsigned int HandledWinMain = 0x004F3F60;
	};

	struct GlobalObjects
	{
		static const unsigned int zCResourceManager = 0x008CE9D0;
		static const unsigned int zCCamera = 0x00873240;
		static const unsigned int oCGame = 0x008DA6BC;
		static const unsigned int zCTimer = 0x008CF1E8;
		static const unsigned int DInput7DeviceMouse = 0x0086D2FC;
		static const unsigned int DInput7DeviceKeyboard = 0x0086D2F0;
		static const unsigned int zCInput = 0x0086CCA0;
		static const unsigned int s_globFreePart = 0x00874374;
		static const unsigned int zCOption = 0x00869694;
	};

	struct zCQuadMark
	{
		static const unsigned int Constructor = 0x005AB810;
		static const unsigned int Destructor = 0x005ABA60;
		static const unsigned int CreateQuadMark = 0x005ACF20;
		static const unsigned int Offset_QuadMesh = 0x34;
		static const unsigned int Offset_Material = 0x3C;
		static const unsigned int Offset_ConnectedVob = 0x38;
		static const unsigned int Offset_DontRepositionConnectedVob = 0x48;
	};

	struct zCRndD3D
	{
		static const unsigned int VidSetScreenMode = 0x0071FC70;
		static const unsigned int DrawLineZ = 0x00716D20;
	};

	struct zCOption
	{
		static const unsigned int ReadInt = 0x0045CDB0;
		static const unsigned int ReadBool = 0x0045CB80;
		static const unsigned int ReadDWORD = 0x0045CEA0;
		static const unsigned int Offset_CommandLine = 0x284;
	};

	struct zFILE
	{
		static const unsigned int Open = 0x0043F450;
	};

	struct zCMesh
	{
		static const unsigned int Offset_Polygons = 0x44;
		static const unsigned int Offset_PolyArray = 0x50;
		static const unsigned int Offset_NumPolygons = 0x34;
		static const unsigned int CreateListsFromArrays = 0x00558AB0;
		static const unsigned int CreateTriMesh = 0x00554440;
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

		static const unsigned int Offset_OverrideColor = 0x558;
		static const unsigned int Offset_OverrideFlag = 0x564;
	};

	struct zCParticleEmitter
	{
		static const unsigned int Offset_VisIsQuadPoly = 0x190;
		static const unsigned int Offset_VisTexture = 0x2DC;
		static const unsigned int Offset_VisAlphaBlendFunc = 0x308;
	};

	struct zCStaticPfxList
	{
		static const unsigned int TouchPFX = 0x0058D590;
	};

	struct zERROR
	{
		// Start/End for problematic SendMessage-Broadcast which causes the game to conflict with other applications
		//static const unsigned int BroadcastStart = 0x0044CB2D;
		//static const unsigned int BroadcastEnd = 0x0044CB3C;
	};

	struct zCCamera
	{
		static const unsigned int GetTransform = 0x00536460;
		static const unsigned int SetTransform = 0x00536300;
		static const unsigned int UpdateViewport = 0x00536850;
		static const unsigned int Activate = 0x005364C0;
		static const unsigned int BBox3DInFrustum = 0x00536EF0;
		
	};

	struct zCVob
	{
		static const unsigned int Offset_WorldMatrixPtr = 0x3C;
		//static const unsigned int Offset_BoundingBoxWS = 0x40;
		static const unsigned int GetVisual = 0x005E9A70;
		static const unsigned int SetVisual = 0x005D6E10;
		static const unsigned int GetPositionWorld = 0x005EE380;
		static const unsigned int Offset_GroundPoly = 0x0AC;
		static const unsigned int DoFrameActivity = 0x006DCC80;
		static const unsigned int GetBBoxLocal = 0x005EDCF0;
		static const unsigned int Offset_HomeWorld = 0x0A8;

		static const unsigned int Offset_Flags = 0xE4;
		static const unsigned int MASK_ShowVisual = 0x1;
		static const unsigned int Offset_CameraAlignment = 0x110;
		static const unsigned int SHIFTLR_CameraAlignment = 0x1E;

		static const unsigned int Destructor = 0x005D32A0;

		static const unsigned int Offset_WorldPosX = 0x48;
		static const unsigned int Offset_WorldPosY = 0x58;
		static const unsigned int Offset_WorldPosZ = 0x68;

		static const unsigned int Offset_SleepingMode = 0xEC;
		static const unsigned int MASK_SkeepingMode = 3;	

		static const unsigned int EndMovement = 0x005F0B60;
	};

	struct zCInput
	{
		static const unsigned int GetDeviceEnabled = 0x004C8760;
		static const unsigned int SetDeviceEnabled = 0x004C8710;
	};

	struct zCVisual
	{
		static const unsigned int VTBL_GetFileExtension = 17;
		static const unsigned int Destructor = 0x005D9F10;
	};

	struct zCBspTree
	{
		static const unsigned int AddVob = 0x0051E6F0;
		static const unsigned int LoadBIN = 0x00525330;
		static const unsigned int Offset_NumPolys = 0x24;
		static const unsigned int Offset_PolyArray = 0x10;
		static const unsigned int Offset_RootNode = 0x8;
		static const unsigned int Offset_WorldMesh = 0xC;
		static const unsigned int Offset_LeafList = 0x18;
		static const unsigned int Offset_NumLeafes = 0x20;
		static const unsigned int Offset_ArrPolygon = 0x4C;
		static const unsigned int Offset_BspTreeMode = 0x58;
		static const unsigned int Render = 0x0051D840;
	};

	struct zCBspBase
	{
		static const unsigned int CollectPolysInBBox3D = 0x005200C0;
		static const unsigned int CheckRayAgainstPolys = 0x0051F180;
		static const unsigned int CheckRayAgainstPolysCache = 0x0051F440;
		static const unsigned int CheckRayAgainstPolysNearestHit = 0x0051F2A0;
		
	};

	struct zCPolygon
	{
		static const unsigned int Offset_VerticesArray = 0x00;
		static const unsigned int Offset_FeaturesArray = 0x2C;
		static const unsigned int Offset_NumPolyVertices = 0x30;
		static const unsigned int Offset_PolyFlags = 0x31;
		static const unsigned int Offset_Material = 0x18;
		static const unsigned int Offset_Lightmap = 0x1C;

		static const unsigned int AllocVerts = 0x00599840;
		static const unsigned int CalcNormal = 0x00596540;

		static const unsigned int Constructor = 0x00595B00;
		static const unsigned int Destructor = 0x00595B30;

		static const unsigned int Size = 0x38;
	};

	struct zCThread
	{
		static const unsigned int SuspendThread = 0x005CE220;
		static const unsigned int Offset_SuspendCount = 0x0C;
	};

	struct zCBspLeaf
	{
		static const unsigned int Size = 0x5C;
	};

	struct zSTRING
	{
		static const unsigned int ToChar = 0x0045E2E0;
	};

	struct zCModelPrototype
	{
		static const unsigned int Offset_NodeList = 0x74;
	};

	struct zCModelTexAniState
	{
		static const unsigned int UpdateTexList = 0x0055D610;
	};

	struct zCMaterial
	{
		static const unsigned int Offset_Texture = 0x34;
		static const unsigned int Offset_AlphaFunc = 0x70;
		static const unsigned int Offset_MatGroup = 0x40;
		static const unsigned int Offset_TexAniCtrl = 0x4C;

		static const unsigned int InitValues = 0x0054D3C0;
		static const unsigned int Constructor = 0x0054CFC0;
		static const unsigned int Destructor = 0x0054D230;
		static const unsigned int GetAniTexture = 0x007154C0;
		
	};

	struct zCObject
	{
		//static const unsigned int Release = 0x0040C310;
		static const unsigned int GetObjectName = 0x0058A160;
	};

	struct zCTexture
	{
		static const unsigned int zCTex_D3DInsertTexture = 0x0071D7D0;
		static const unsigned int LoadResourceData = 0x005CA5E0;
		static const unsigned int Offset_CacheState = 0x4C;
		static const unsigned int Mask_CacheState = 3;
		//static const unsigned int GetName = 0x0059AF90;
		//static const unsigned int GetAniTexture = 0x005DA9B0;

		static const unsigned int Offset_Flags = 0x88;
		static const unsigned int Mask_FlagHasAlpha = 0x1;
		static const unsigned int Mask_FlagIsAnimated = 0x2;
		
		static const unsigned int zCResourceTouchTimeStamp = 0x005B5500;
		static const unsigned int zCResourceTouchTimeStampLocal = 0x005B5580;

		static const unsigned int Offset_Surface = 0xD4;
	};

	struct zCBinkPlayer
	{
		static const unsigned int GetPixelFormat = 0x0043C9E0;
		static const unsigned int OpenVideo = 0x00424E90;
		static const unsigned int Offset_VideoHandle = 0x30;
	};

	struct oCSpawnManager
	{
		static const unsigned int SpawnNpc = 0x006D0710;

	};

	struct zCDecal
	{
		static const unsigned int Offset_DecalSettings = 0x34;
		//static const unsigned int GetAlphaTestEnabled = 0x00556BE0;
	};

	struct zCResourceManager
	{
		static const unsigned int CacheIn = 0x005B5CB0;
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
		static const unsigned int Render = 0x005F3EC0;
		static const unsigned int LoadWorld = 0x005F8A00;
		static const unsigned int VobRemovedFromWorld = 0x005F64C0;
		static const unsigned int VobAddedToWorld = 0x005F6360;
		static const unsigned int Offset_GlobalVobTree = 0x24;
		static const unsigned int Call_Render_zCBspTreeRender = 0x005F3F95;
		//static const unsigned int GetActiveSkyController = 0x006060A0;
		static const unsigned int Offset_SkyControllerOutdoor = 0x0D0;
		static const unsigned int DisposeVobs = 0x005F55F0;
		static const unsigned int Offset_BspTree = 0x198;
	};

	struct oCWorld
	{
		static const unsigned int InsertVobInWorld = 0x006D7120;
		static const unsigned int EnableVob = 0x006D7130;
		static const unsigned int DisableVob = 0x006D7250;
		static const unsigned int RemoveFromLists = 0x006D7750;
	};

	struct zCBspNode
	{
		/*static const unsigned int RenderIndoor = 0x0052A3E0;
		static const unsigned int RenderOutdoor = 0x0052A7B0;
		static const unsigned int REPL_RenderIndoorEnd = 0x0052A47D;
		
		static const unsigned int REPL_RenderOutdoorStart = 0x0052A7B0;
		static const unsigned int REPL_RenderOutdoorEnd = 0x0052A9BA;
		static const unsigned int SIZE_RenderOutdoor = 0x252;*/

	};

	struct oCNPC
	{
		static const unsigned int ResetPos = 0x0074CED0;
		static const unsigned int Enable = 0x006A2000;
		static const unsigned int Disable = 0x006A1D20;		
		static const unsigned int InitModel = 0x00695020;
		static const unsigned int IsAPlayer = 0x0069E9D0;
	};

	struct oCGame
	{
		static const unsigned int EnterWorld = 0x0063EAD0;
		static const unsigned int TestKeys = 0x00660000;
		static const unsigned int Var_Player = 0x008DBBB0;		
	};

	struct zCView
	{
		static const unsigned int SetMode = 0x00702180;
		static const unsigned int REPL_SetMode_ModechangeStart = 0x007021A9;
		static const unsigned int REPL_SetMode_ModechangeEnd = 0x0007021B8;
	};

	struct zCVobLight
	{
		static const unsigned int Offset_LightColor = 0x120; // Right before Range
		static const unsigned int Offset_Range = 0x124;
		static const unsigned int Offset_LightInfo =  0x144;
		static const unsigned int Mask_LightEnabled = 0x20;
		static const unsigned int DoAnimation = 0x005DB820;
	};

	struct zCMorphMesh
	{
		static const unsigned int Offset_MorphMesh = 0x38;
		static const unsigned int Offset_TexAniState = 0x40;
		static const unsigned int AdvanceAnis = 0x00586E90;
		static const unsigned int CalcVertexPositions = 0x00586AE0;
		
	};

	struct zCParticleFX
	{
		static const unsigned int Offset_FirstParticle = 0x34;
		static const unsigned int Offset_TimeScale = 0x8C;
		static const unsigned int Offset_LocalFrameTimeF = 0x90; // Offset_TimeScale + 4
		static const unsigned int Offset_PrivateTotalTime = 0x84; // Offset_TimeScale - 8
		static const unsigned int Offset_LastTimeRendered = 0x88;
		static const unsigned int Offset_Emitters = 0x54;

		static const unsigned int OBJ_s_pfxList = 0x0089B2A0;
		static const unsigned int OBJ_s_partMeshQuad = 0x0089B2BC;
		static const unsigned int CheckDependentEmitter = 0x005913C0;
		static const unsigned int UpdateParticle = 0x0058F4C0;

		//static const unsigned int OBJ_s_pfxList = 0x008D9214;

		static const unsigned int CreateParticlesUpdateDependencies = 0x0058F210;
		static const unsigned int SetVisualUsedBy = 0x0058DD60;
		static const unsigned int Destructor = 0x0058D340;
		static const unsigned int UpdateParticleFX = 0x0058F130;
	};

	struct zCModel
	{
		static const unsigned int RenderNodeList = 0x0055FA50;
		static const unsigned int UpdateAttachedVobs = 0x005667F0;
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
		static const unsigned int GetVisualName = 0x00563EF0;
	};
};