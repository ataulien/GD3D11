#pragma once

/** Holds all memorylocations for the selected game */
struct GothicMemoryLocations
{
	struct Functions
	{
		static const unsigned int Alg_Rotation3DNRad = 0x00517E90;
		static const unsigned int vidGetFPSRate = 0x004FDCD0;
		static const unsigned int HandledWinMain = 0x00502ED0;
		//static const unsigned int ExitGameFunc = 0x00425F30;
		static const unsigned int zCExceptionHandler_UnhandledExceptionFilter = 0x004C88C0;
	}; 

	struct zCQuadMark
	{
		static const unsigned int Constructor = 0x005D0970;
		static const unsigned int Destructor = 0x005D0B90;
		static const unsigned int CreateQuadMark = 0x005D2030;
		static const unsigned int Offset_QuadMesh = 0x34;
		static const unsigned int Offset_Material = 0x3C;
		static const unsigned int Offset_ConnectedVob = 0x38;
		static const unsigned int Offset_DontRepositionConnectedVob = 0x48;
	};

	struct zCThread
	{
		static const unsigned int SuspendThread = 0x005F9370;
		static const unsigned int Offset_SuspendCount = 0x0C;
	};

	struct zCBinkPlayer
	{
		static const unsigned int GetPixelFormat = 0x00440790;
		static const unsigned int OpenVideo = 0x0043E0F0;
		static const unsigned int Offset_VideoHandle = 0x30;
	};

	struct CGameManager
	{
		static const unsigned int ExitGame = 0x00425780;
	};

	struct zCOption
	{
		static const unsigned int ReadInt = 0x00462390;
		static const unsigned int ReadBool = 0x00462160;
		static const unsigned int Offset_CommandLine = 0x284;
	};

	struct zCRndD3D
	{
		static const unsigned int VidSetScreenMode = 0x00658BA0;
		static const unsigned int DrawLineZ = 0x0064DB00;
	};

	struct zERROR
	{
		// Start/End for problematic SendMessage-Broadcast which causes the game to conflict with other applications
		static const unsigned int BroadcastStart = 0x0044C5D5;
		static const unsigned int BroadcastEnd = 0x0044C5E4;
	};

	struct oCGame
	{
		static const unsigned int EnterWorld = 0x006C96F0;
		static const unsigned int TestKeys = 0x006FD560;
		static const unsigned int Var_Player = 0x00AB2684;
	};

	struct oCNPC
	{
		static const unsigned int ResetPos = 0x006824D0;
		static const unsigned int InitModel = 0x00738480;
	};

	struct zCView
	{
		static const unsigned int SetMode = 0x007ABDB0;
		static const unsigned int REPL_SetMode_ModechangeStart = 0x007ABDD9;
		static const unsigned int REPL_SetMode_ModechangeEnd = 0x007ABDE8;
	};

	struct zCObject
	{
		//static const unsigned int Release = 0x0040C310;
		static const unsigned int GetObjectName = 0x005A9CD0;
	};

	struct zCSkyController_Outdoor
	{
		static const unsigned int OBJ_ActivezCSkyController = 0x0099AC8C;

		static const unsigned int Offset_Sun = 0x5F4; // First of the two planets
		static const unsigned int Offset_MasterTime = 0x80;
		static const unsigned int Offset_MasterState = 0x88;
		static const unsigned int Offset_SkyLayer1 = 0x5A8;
		static const unsigned int Offset_SkyLayer2 = 0x5C4;
		static const unsigned int Offset_SkyLayerState = 0x5A0;		
		static const unsigned int Offset_SkyLayerState0 = 0x120;
		static const unsigned int Offset_SkyLayerState1 = 0x124;
		static const unsigned int Offset_OverrideColor = 0x558;
		static const unsigned int Offset_OverrideFlag = 0x564;
		static const unsigned int Interpolate = 0x005E8C20;
		static const unsigned int Offset_InitDone = 0x7C;
		static const unsigned int Init = 0x005E6A00;
		static const unsigned int GetUnderwaterFX = 0x005E0050;
	};

	struct zCSkyController
	{
		static const unsigned int VTBL_RenderSkyPre = 27; //0x6C / 4
		static const unsigned int VTBL_RenderSkyPost = (0x78 / 4);
	};

	struct zCParticleEmitter
	{
		static const unsigned int Offset_VisTexture = 0x318;
		static const unsigned int Offset_VisAlignment = 0x31C;
		static const unsigned int Offset_VisAlphaBlendFunc = 0x340;
		static const unsigned int Offset_AlphaDist = 0x35C;
		static const unsigned int Offset_VisAlphaStart = 0x1F0;
		static const unsigned int Offset_VisTexAniIsLooping = 0x198;
		static const unsigned int Offset_VisIsQuadPoly = 0x190;
		
	};

	struct zCMesh
	{
		static const unsigned int Offset_Polygons = 0x44;
		static const unsigned int Offset_NumPolygons = 0x34;
	};

	struct zCParticleFX
	{
		static const unsigned int Offset_FirstParticle = 0x34;
		static const unsigned int Offset_TimeScale = 0x8C;
		static const unsigned int Offset_LocalFrameTimeF = 0x90; // Offset_TimeScale + 4
		static const unsigned int Offset_PrivateTotalTime = 0x84; // Offset_TimeScale - 8
		static const unsigned int Offset_Emitters = 0x54;
		static const unsigned int Offset_LastTimeRendered = 0x88;

		static const unsigned int Offset_ConnectedVob = 0x70;
		static const unsigned int UpdateParticle = 0x005AF500;
		static const unsigned int OBJ_s_pfxList = 0x008D9214;
		static const unsigned int OBJ_s_partMeshQuad = 0x008D9230;
		static const unsigned int CreateParticlesUpdateDependencies = 0x005AF240;
		static const unsigned int SetVisualUsedBy = 0x005B1DD0;
		static const unsigned int GetNumParticlesThisFrame = 0x005B1A90;
		static const unsigned int UpdateParticleFX = 0x005AF160;
		static const unsigned int CheckDependentEmitter = 0x005B1C30;

		static const unsigned int Offset_PrevPFX = 0x80; // PrivateTotalTime - 4
		static const unsigned int Offset_NextPFX = 0x7C; // PrivateTotalTime - 8
		static const unsigned int Destructor = 0x005AD0E0;
	};

	struct zCStaticPfxList
	{
		static const unsigned int TouchPFX = 0x005AD330;
	};

	struct zCInput
	{
		static const unsigned int GetDeviceEnabled = 0x004D5160;
		static const unsigned int SetDeviceEnabled = 0x004D5100;
	};

	struct GlobalObjects
	{
		static const unsigned int zCResourceManager = 0x0099AB30;
		static const unsigned int zCCamera = 0x008D7F94;
		static const unsigned int oCGame = 0x00AB0884;
		static const unsigned int zCTimer = 0x0099B3D4;
		static const unsigned int s_globFreePart = 0x008D9228;
		static const unsigned int DInput7DeviceMouse = 0x008D1D70;
		static const unsigned int DInput7DeviceKeyboard = 0x008D1D64;
		static const unsigned int zCInput = 0x008D1650;
		static const unsigned int zCOption = 0x008CD988;
		static const unsigned int zCParser = 0xAB40C0;
	};

	struct zCParser
	{
		static const unsigned int CallFunc = 0x7929F0;
	};

	struct zCMorphMesh
	{
		static const unsigned int Offset_MorphMesh = 0x38;
		static const unsigned int Offset_TexAniState = 0x40;
		static const unsigned int AdvanceAnis = 0x005A6830;
		static const unsigned int CalcVertexPositions = 0x005A6480;
	};

	struct zCMeshSoftSkin
	{
		//static const unsigned int Offset_NodeIndexList = 0x38C;
		static const unsigned int Offset_VertWeightStream = 0x0FC;
	};

	struct zCModelPrototype
	{
		static const unsigned int Offset_NodeList = 0x64;
	};

	struct zCBspLeaf
	{
		static const unsigned int Size = 0x5C;
	};

	struct zCVobLight
	{
		static const unsigned int Offset_LightColor = 0x140; // Right before Range
		static const unsigned int Offset_Range = 0x144;
		static const unsigned int Offset_LightInfo =  0x164;
		static const unsigned int Mask_LightEnabled = 0x20;
		static const unsigned int DoAnimation = 0x006081C0;
	};

	struct zFILE
	{
		static const unsigned int Open = 0x00448D30;
	};

	struct zCBspNode
	{
		static const unsigned int RenderIndoor = 0x0052F0E0;
		static const unsigned int RenderOutdoor = 0x0052F490;

		static const unsigned int REPL_RenderIndoorEnd = 0x0052F153;

		static const unsigned int REPL_RenderOutdoorStart = 0x0052F495;
		static const unsigned int REPL_RenderOutdoorEnd = 0x0052F69E;
		static const unsigned int SIZE_RenderOutdoor = 0x211;

	};

	struct zCModelTexAniState
	{
		static const unsigned int UpdateTexList = 0x00576DA0;
	};

	struct zCModel
	{
		static const unsigned int RenderNodeList = 0x00579560;
		static const unsigned int UpdateAttachedVobs = 0x00580900;
		static const unsigned int Offset_ModelProtoList = 0x64;
		static const unsigned int Offset_NodeList = 0x70;
		static const unsigned int Offset_MeshSoftSkinList = 0x7C;
		static const unsigned int Offset_MeshLibList = 0xBC;
		static const unsigned int Offset_AttachedVobList = 0x98;
		static const unsigned int AdvanceAnis = 0x0057CA90;
		static const unsigned int SIZE_AdvanceAnis = 0x119E;
		static const unsigned int RPL_AniQuality = 0x0057CCC1;
		static const unsigned int Offset_DrawHandVisualsOnly = 0x174;
		static const unsigned int Offset_ModelFatness = 0x128;
		static const unsigned int Offset_ModelScale = 0x12C;
		static const unsigned int Offset_Flags = 0x1F8;
		static const unsigned int Offset_DistanceModelToCamera = 0x120;
		static const unsigned int GetVisualName = 0x0057DF60;
	};

	struct zCCamera
	{
		static const unsigned int GetTransform = 0x0054A6A0;
		static const unsigned int SetTransform = 0x0054A540;
		static const unsigned int UpdateViewport = 0x0054AA90;
		static const unsigned int Activate = 0x0054A700;
		static const unsigned int Offset_NearPlane = 0x900;
		static const unsigned int Offset_FarPlane = 0x8FC;
		static const unsigned int SetFarPlane = 0x0054B200;
		static const unsigned int BBox3DInFrustum = 0x0054B410;
		static const unsigned int Var_FreeLook = 0x008CE42C;
		static const unsigned int SetFOV = 0x0054A960;
	};

	struct zCProgMeshProto
	{
		static const unsigned int Offset_PositionList = 0x34;
		static const unsigned int Offset_NormalsList = 0x38;
		static const unsigned int Offset_Submeshes = 0xA4;
		static const unsigned int Offset_NumSubmeshes = 0xA8;
	};

	struct zCVob
	{
		static const unsigned int Offset_WorldMatrixPtr = 0x3C;
		//static const unsigned int Offset_BoundingBoxWS = 0x40;
		static const unsigned int GetVisual = 0x00616B20;
		static const unsigned int SetVisual = 0x006024F0;
		static const unsigned int GetPositionWorld = 0x0052DC90;
		static const unsigned int GetBBoxLocal = 0x0061B1F0;
		static const unsigned int Offset_HomeWorld = 0x0B8;
		static const unsigned int Offset_GroundPoly = 0x0BC;
		static const unsigned int Offset_Flags = 0x104;
		static const unsigned int MASK_ShowVisual = 0x1;
		static const unsigned int Offset_CameraAlignment = 0x110;
		static const unsigned int SHIFTLR_CameraAlignment = 0x1E;

		static const unsigned int Destructor = 0x005FE470;

		static const unsigned int Offset_WorldPosX = 0x48;
		static const unsigned int Offset_WorldPosY = 0x58;
		static const unsigned int Offset_WorldPosZ = 0x68;

		
	};

	struct zCVisual
	{
		static const unsigned int VTBL_GetFileExtension = 17;
		static const unsigned int Destructor = 0x00606800;
	};

	

	struct zCDecal
	{
		static const unsigned int Offset_DecalSettings = 0x34;
		static const unsigned int GetAlphaTestEnabled = 0x00556BE0;
	};

	struct oCSpawnManager
	{
		static const unsigned int SpawnNpc = 0x00778E70;
	};


	struct zCBspTree
	{
		static const unsigned int AddVob = 0x00531040;
		static const unsigned int LoadBIN = 0x00538E10;
		static const unsigned int Offset_NumPolys = 0x24;
		static const unsigned int Offset_PolyArray = 0x10;
		static const unsigned int Offset_WorldMesh = 0xC;

		static const unsigned int Offset_RootNode = 0x8;
		static const unsigned int Offset_LeafList = 0x18;
		static const unsigned int Offset_NumLeafes = 0x20;
		static const unsigned int Offset_BspTreeMode = 0x58;

		static const unsigned int Render = 0x00530080;
		static const unsigned int SIZE_Render = 0xA48;

		

		static const unsigned int CALL_RenderOutdoor = 0x005307E9;
		static const unsigned int CALL_RenderOutdoor2 = 0x005308C1;
		static const unsigned int CALL_RenderIndoor = 0x0053041C;
		static const unsigned int CALL_RenderIndoor2 = 0x00530432;
		static const unsigned int CALL_RenderIndoor3 = 0x00530490;
		static const unsigned int CALL_RenderIndoor4 = 0x005304A2;
		static const unsigned int CALL_RenderTrivIndoor = 0x005304AB;
	};

	struct zCPolygon
	{
		static const unsigned int Offset_VerticesArray = 0x00;
		static const unsigned int Offset_FeaturesArray = 0x2C;
		static const unsigned int Offset_NumPolyVertices = 0x30;
		static const unsigned int Offset_PolyFlags = 0x31;
		static const unsigned int Offset_Material = 0x18;
		static const unsigned int Offset_Lightmap = 0x1C;
	};

	struct zSTRING
	{
		static const unsigned int ToChar = 0x004639D0;
	};

	struct zCMaterial
	{
		static const unsigned int Offset_Texture = 0x34;
		static const unsigned int Offset_AlphaFunc = 0x74;
		static const unsigned int Offset_MatGroup = 0x40;
		static const unsigned int Offset_TexAniCtrl = 0x4C;
	
		static const unsigned int InitValues = 0x00564260;
		static const unsigned int Constructor = 0x00563E00;
		static const unsigned int Destructor = 0x00564070;
		static const unsigned int GetAniTexture = 0x0064BA20;
	
	};

	struct zCTexture
	{
		static const unsigned int zCTex_D3DInsertTexture = 0x00656120;
		static const unsigned int LoadResourceData = 0x005F54D0;
		static const unsigned int GetName = 0x005A9CD0;
		static const unsigned int Offset_CacheState = 0x4C;
		static const unsigned int Mask_CacheState = 3;

		static const unsigned int Offset_Flags = 0x88;
		static const unsigned int Mask_FlagHasAlpha = 0x1;
		static const unsigned int Mask_FlagIsAnimated = 0x2;

		static const unsigned int zCResourceTouchTimeStamp = 0x005DC810;
		static const unsigned int zCResourceTouchTimeStampLocal = 0x005DC890;

		static const unsigned int Load = 0x005F36A0;

		static const unsigned int Offset_Surface = 0xD4;
		static const unsigned int XTEX_BuildSurfaces = 0x006552A0;
		//static const unsigned int Load = 0x005F4360;
	};

	struct zCResourceManager
	{
		static const unsigned int CacheIn = 0x005DD040;
		static const unsigned int CacheOut = 0x005DD350;
	};

	struct oCWorld
	{
		//static const unsigned int InsertVobInWorld = 0x006D7120;
		static const unsigned int EnableVob = 0x00780340;
		static const unsigned int DisableVob = 0x00780460;
		static const unsigned int RemoveFromLists = 0x00780990;
	};

	struct zCWorld
	{
		static const unsigned int Render = 0x00621700;
		static const unsigned int InsertVobInWorld = 0x00780330;
		static const unsigned int VobAddedToWorld = 0x00624830;
		static const unsigned int Call_Render_zCBspTreeRender = 0x00621830;
		static const unsigned int Offset_GlobalVobTree = 0x24;
		static const unsigned int LoadWorld = 0x006270D0;
		static const unsigned int VobRemovedFromWorld = 0x00624970;
		static const unsigned int Offset_SkyControllerOutdoor = 0x0E4;
		static const unsigned int GetActiveSkyController = 0x006203A0;
		static const unsigned int DisposeWorld = 0x00623D30;
		static const unsigned int DisposeVobs = 0x00623960;
	};
};