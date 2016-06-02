#pragma once
#include "pch.h"
#include "WorldConverter.h"

class BaseVertexBuffer;
class BaseTexture;
class BaseConstantBuffer;
class BaseLineRenderer;
class BasePShader;
class BaseVShader;
class BaseHDShader;
class BaseGShader;

struct VobInfo;

struct DisplayModeInfo
{
	DWORD Height;
	DWORD Width;
	DWORD Bpp;
};

enum RenderStage
{
	STAGE_DRAW_WORLD = 0,
	STAGE_DRAW_SKELETAL = 1,
};

struct ViewportInfo
{
	ViewportInfo(){}
	ViewportInfo(	unsigned int topleftX, 
		unsigned int topleftY, 
		unsigned int width, 
		unsigned int height, 
		float minZ = 0.0f, 
		float maxZ = 1.0f)
	{
		TopLeftX = topleftX;
		TopLeftY = topleftY;
		Width = width;
		Height = height;
		MinZ = minZ;
		MaxZ = maxZ;
	}

	unsigned int TopLeftX;
	unsigned int TopLeftY;
	unsigned int Width;
	unsigned int Height;
	float MinZ;
	float MaxZ;
};

typedef void (__cdecl* stateBoundCallback)(void*);

struct PipelineState
{
	enum EDrawCallType
	{
		DCT_DrawTriangleList,
		DCT_DrawIndexed,
		DCT_DrawInstanced,
		DCT_DrawIndexedInstanced
	};

	enum ETransparencyMode
	{
		TM_NONE = 0,
		TM_MASKED = 1,
		TM_BLEND = 2
	};

	enum EShaderStage
	{
		SS_VERTEX,
		SS_PIXEL,
		SS_GEOMETRY,
		SS_HULLDOMAIN
	};

	PipelineState()
	{
		StateVersion = 0;
		BoundCallbackUserdata = NULL;
		BoundCallback = NULL;
	}

	/** Sets the constantbuffers for the given shaderstage */
	virtual void SetConstantBuffers(const std::vector<BaseConstantBuffer*>& cbs, EShaderStage stage)=0;

	virtual ~PipelineState(){}



	/** Sets the state up for instanced rendering */
	virtual void SetupInstancing(BaseVertexBuffer* InstanceData, unsigned int baseInstanceLocation = 0)=0;

	/** Constructs a state out of the given dataset */
	virtual void MakeState(WorldMeshInfo* data, zCMaterial* material, unsigned int frame)=0;

	/** Constructs a state out of the given dataset */
	virtual void MakeState(MeshInfo* data, zCMaterial* material, unsigned int frame)=0;

	/** Last frame this state was accurate on. If this state is
	outdated, it should be renewed by using the visual-objects for example */
	unsigned int StateVersion;

	/** Number if instances this state should render */
	// FIXME: This shouldn't really be in this base object, but implementation get's hard if not
	unsigned int NumInstances;

	/** Callbackfunction to trigger when this state is being bound */
	void* BoundCallbackUserdata;
	stateBoundCallback BoundCallback;
};