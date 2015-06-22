#pragma once
#include "pch.h"

/** LEGACY */
#define SET(TYPE, NAME) void Set##NAME (const TYPE& value) { NAME = value; }
#define GET(TYPE, NAME) const TYPE& Get##NAME () const { return NAME; }

#define SET_PTR(TYPE, NAME) void Set##NAME (TYPE value) { NAME = value; }
#define GET_PTR(TYPE, NAME) TYPE Get##NAME () { return NAME; }

#define GETSET(TYPE, NAME) GET(TYPE,NAME); SET(TYPE,NAME);
#define GETSET_PTR(TYPE, NAME) GET_PTR(TYPE,NAME); SET_PTR(TYPE,NAME);

#define GETSET_MEMBER(TYPE, NAME) public: GETSET(TYPE,NAME); private: TYPE NAME;
#define GETSET_MEMBER_PTR(TYPE, NAME) public: GETSET_PTR(TYPE,NAME); private: TYPE NAME;

#define GETSET_MEMBER_PROT(TYPE, NAME) public: GETSET(TYPE,NAME); protected: TYPE NAME;
#define GETSET_MEMBER_PROT_PTR(TYPE, NAME) public: GETSET_PTR(TYPE,NAME); protected: TYPE NAME;

#define GET_LIST_PAIR(LIST, NAME) std::pair<LIST::iterator, LIST::iterator> Get##NAME##Iterators() { return std::make_pair(NAME.begin(),NAME.end());	}
/** LEGACY **/

/** Objects which can draw a primitive only out of wireframe-lines. It also is able to intesect them */

struct LineVertex;
class D3D11PShader;
class EditorLinePrimitive
{
public:
	EditorLinePrimitive(void);
	virtual ~EditorLinePrimitive(void);

	/** Creates the buffers (Polygon-version) and sets up the rest of the object */
	HRESULT CreateSolidPrimitive(LineVertex* PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/** Creates the buffers (line-version) and sets up the rest of the object */
	HRESULT CreatePrimitive(LineVertex* PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	/** Creates a circle of lines. Axis: 0=xz 1=yx 2=yz*/
	HRESULT CreateCirclePrimitive(float Radius, UINT Detail, const D3DXVECTOR4* Color, int Axis = 0);

	/** Creates a plate, not of lines. Can't use intersection on this*/
	HRESULT CreateFilledCirclePrimitive(float Radius, UINT Detail, const D3DXVECTOR4* Color, int Axis = 0);

	/** Creates a ball of lines.*/
	HRESULT CreateLineBallPrimitive(UINT Detail, const D3DXVECTOR4* Color);

	/** Creates an arrow of lines */
	HRESULT CreateArrowPrimitive(const D3DXVECTOR4* Color, float ArrowRadius = 0.25, float ArrowOffset = 0.75);

	/** Creates a cone of lines */
	HRESULT CreateSimpleConePrimitive(float Length, float Radius, UINT Detail, const D3DXVECTOR4* Color);

	/** Creates a box of lines */
	HRESULT CreateLineBoxPrimitive(D3DXVECTOR4* Color);

	/** Creates a solid box */
	HRESULT CreateSolidBoxPrimitive(D3DXVECTOR4* Color, float Extends);

	/** Creates a grid of lines (1x1)*/
	HRESULT CreateLineGrid(int LinesX,int LinesY,D3DXVECTOR2* Middle, D3DXVECTOR4* Color);

	/** Sets the shader to render with */
	void SetShader(D3D11PShader* Shader);
	void SetSolidShader(D3D11PShader* SolidShader);

	/** Sets the transforms */
	void SetLocation(const D3DXVECTOR3& NewLoc);
	void SetRotation(const D3DXVECTOR3& NewRotation);
	void SetScale(const D3DXVECTOR3& NewScale);
	void SetWorldMatrix(const D3DXMATRIX* World,const D3DXVECTOR3* Loc,const D3DXVECTOR3* Rot,const D3DXVECTOR3* Scale);

	/** Renders the primitive */
	HRESULT RenderPrimitive(int Pass=-1);

	/** If set to true, the primitive will ignore all drawcalls automatically */
	void SetHidden(bool bIsHidden)
	{
		bHidden = bIsHidden;
	}

	/** Intersects the whole primitive. If hit, it returns a distance other than -1 */
	float IntersectPrimitive(D3DXVECTOR3* RayOrigin, D3DXVECTOR3* RayDirection, float Epsilon = 0.01);

	bool IntersectTriangle( const D3DXVECTOR3* orig, const D3DXVECTOR3* dir,
                        D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                        FLOAT* t, FLOAT* u, FLOAT* v );

	/** Puts the color into the Normal and TexCoord channels */
	static void EncodeColor(LineVertex* vx, const D3DXVECTOR4* Color);

	GETSET(D3DXMATRIX, RotationMatrix);
	GETSET(D3DXVECTOR3, RotationMatrixAngles);
	

	GET_PTR(D3D11PShader*, PrimShader);
	GET_PTR(D3D11PShader*, SolidPrimShader);
	GETSET_MEMBER(bool, bLocalRotation);
	GETSET_MEMBER(bool, bJustUseRotationMatrix); // If true we will just multiply the existing rotation matrix when creating a new world matrix
private:

	/** Deletes all content */
	void DeleteContent();

	/** Recalculates the world matrix */
	void RecalcTransforms();

	/** Intersects only one line segment */
	float IntersectLineSegment(const D3DXVECTOR3* rayOrigin,const D3DXVECTOR3* rayVec,const D3DXVECTOR3* lineStart,const D3DXVECTOR3* lineEnd, float Epsilon);

	

	/** Renders a vertexbuffer with the given shader */
	void RenderVertexBuffer(ID3D11Buffer* VB, UINT NumVertices, D3D11PShader* Shader, D3D11_PRIMITIVE_TOPOLOGY Topology, int Pass = -1);

	/** The bunch of vertices we have */
	LineVertex* Vertices;
	UINT NumVertices;

	/** Vertex buffer */
	ID3D11Buffer* PrimVB;

	/** Primitives shaders */
	D3D11PShader* PrimShader;
	D3D11PShader* SolidPrimShader;


	/** Solid vertices we have */
	LineVertex* SolidVertices;
	UINT NumSolidVertices;
	ID3D11Buffer* SolidPrimVB;

	/** Transforms */
	D3DXVECTOR3 Location;
	D3DXVECTOR3 Rotation;
	D3DXVECTOR3 Scale;

	/** Transform matrix */
	D3DXMATRIX WorldMatrix;

	/* Matrix for rotation and it's angles */
	D3DXMATRIX RotationMatrix;
	D3DXVECTOR3 RotationMatrixAngles;

	/** If true, ignore drawcalls */
	bool bHidden;

	/** Line list or line strip? */
	D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
	D3D11_PRIMITIVE_TOPOLOGY SolidPrimitiveTopology;
};

