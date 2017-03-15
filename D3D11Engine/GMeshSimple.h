#pragma once
#include "pch.h"
#include "D3D11VertexBuffer.h"

class GMeshSimple
{
public:
	GMeshSimple(void);
	virtual ~GMeshSimple(void);

	/** Load a mesh from file */
	XRESULT LoadMesh(const std::string& file);

	/** Draws all buffers this holds */
	void DrawMesh() const;

	/** Draws a batch of instances */
	void DrawBatch(D3D11VertexBuffer* instances, int numInstances, int instanceDataStride) const;

private:
	D3D11VertexBuffer* VertexBuffer;
	D3D11VertexBuffer* IndexBuffer;
	unsigned int NumVertices;
	unsigned int NumIndices;
};

