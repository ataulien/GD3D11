#pragma once
#include "baselinerenderer.h"

class D3D11VertexBuffer;
class D3D11LineRenderer :
	public BaseLineRenderer
{
public:
	D3D11LineRenderer(void);
	~D3D11LineRenderer(void);

	/** Adds a line to the list */
	virtual XRESULT AddLine(const LineVertex& v1, const LineVertex& v2);

	/** Flushes the cached lines */
	virtual XRESULT Flush();

	/** Clears the line cache */
	virtual XRESULT ClearCache();


private:
	/** Line cache */
	std::vector<LineVertex> LineCache;

	/** Buffer to hold the lines on the GPU */
	D3D11VertexBuffer* LineBuffer;
	unsigned int LineBufferSize; // Size in elements the line buffer can hold
};

