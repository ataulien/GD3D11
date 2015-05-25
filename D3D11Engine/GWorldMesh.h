#pragma once
#include "pch.h"
#include "WorldConverter.h"
#include "BaseGraphicsEngine.h"

class GWorldMesh
{
public:
	GWorldMesh(zCBspTree* bspTree);
	virtual ~GWorldMesh(void);

	/** Draws the worldmesh */
	virtual void DrawMesh();

	/** Returns a reference to the loaded sections */
	std::map<int, std::map<int, WorldMeshSectionInfo>>& GetWorldSections();

	/** Outputs a list of all visible sections in the view-frustum */
	void CollectVisibleSections(std::list<WorldMeshSectionInfo*>& sections);

protected:
	/** Loaded game sections */
	std::map<int, std::map<int, WorldMeshSectionInfo>> WorldSections;

	/** All sections in one vertexbuffer */
	MeshInfo* WrappedWorldMesh;

	/** Transforms-Constantbuffer for the worldmesh */
	BaseConstantBuffer* TransformsCB;

	/** Pipelinestate to render this with */
	std::vector<PipelineState*> PipelineStates;
};

