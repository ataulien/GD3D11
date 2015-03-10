#pragma once
#include "pch.h"
#include "BaseVertexBuffer.h"

class GMeshSimple
{
public:
	GMeshSimple(void);
	virtual ~GMeshSimple(void);

	/** Load a mesh from file */
	XRESULT LoadMesh(const std::string& file);

	/** Draws all buffers this holds */
	void DrawMesh();

	/** Draws a batch of instances */
	void DrawBatch(BaseVertexBuffer* instances, int numInstances, int instanceDataStride);

private:
	BaseVertexBuffer* VertexBuffer;
	BaseVertexBuffer* IndexBuffer;
	unsigned int NumVertices;
	unsigned int NumIndices;
};

