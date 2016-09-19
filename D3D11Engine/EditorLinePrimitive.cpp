#include "pch.h"
#include "EditorLinePrimitive.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "D3D11PShader.h"
#include "BaseLineRenderer.h"
#include "GothicAPI.h"
#include "D3D11ShaderManager.h"

EditorLinePrimitive::EditorLinePrimitive(void)
{
	Vertices = NULL;
	PrimVB = NULL;
	PrimShader = NULL;
	Location = D3DXVECTOR3(0, 0, 0);
	Rotation = D3DXVECTOR3(0, 0, 0);
	Scale = D3DXVECTOR3(1, 1, 1);
	RecalcTransforms();
	bHidden=false;

	SolidPrimShader = NULL;
	SolidVertices = NULL;
	NumSolidVertices = 0;
	SolidPrimVB = NULL;

	NumVertices = 0;

	RotationMatrixAngles=D3DXVECTOR3(0,0,0);
	D3DXMatrixIdentity(&RotationMatrix);
	bLocalRotation=false;

	bJustUseRotationMatrix=false;

	SetSolidShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetShaderManager()->GetPShader("PS_Lines"));
	SetShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetShaderManager()->GetPShader("PS_Lines"));
}


EditorLinePrimitive::~EditorLinePrimitive(void)
{
	delete[] Vertices;
	delete[] SolidVertices;
	if(PrimVB)PrimVB->Release();
	if(SolidPrimVB)SolidPrimVB->Release();
}

/** Deletes all content */
void EditorLinePrimitive::DeleteContent()
{
	delete[] Vertices;
	delete[] SolidVertices;
	if(PrimVB)PrimVB->Release();
	if(SolidPrimVB)SolidPrimVB->Release();
}

/** Creates a grid of lines */
HRESULT EditorLinePrimitive::CreateLineGrid(int LinesX,int LinesY,D3DXVECTOR2* Middle, D3DXVECTOR4* Color)
{
	HRESULT hr;
	LineVertex* vx = new LineVertex[(LinesX+1)*(LinesY+1)*4];
	UINT CurVertex=0;

	//Fill X based lines
	float x;
	float SpacingX=(1.0f/LinesX);
	for(x= -0.5; x<=0.5f+SpacingX; x+=SpacingX)
	{

		vx[CurVertex].Position=D3DXVECTOR3(x,0,-0.5)+D3DXVECTOR3(Middle->x,0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
		vx[CurVertex].Position=D3DXVECTOR3(x,0,0.5)+D3DXVECTOR3(Middle->x,0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
	}

	//Fill Z based lines
	float z;
	float SpacingY=(1.0f/LinesY);
	for(z= -0.5; z<=0.5f+SpacingY; z+=SpacingY)
	{
		vx[CurVertex].Position=D3DXVECTOR3(-0.5,0,z)+D3DXVECTOR3(Middle->x,0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
		vx[CurVertex].Position=D3DXVECTOR3(0.5,0,z)+D3DXVECTOR3(Middle->x,0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
	}


	LE(CreatePrimitive(vx, (LinesX+1)*(LinesY+1)*4));

	delete[] vx;
	return hr;
}

/** Creates a box of lines */
HRESULT EditorLinePrimitive::CreateLineBoxPrimitive(D3DXVECTOR4* Color)
{
	LineVertex vx[24];

	// Bottom
	vx[0].Position = D3DXVECTOR3(-1,-1,-1);
	EncodeColor(&vx[0], Color);
	vx[1].Position = D3DXVECTOR3(1,-1,-1);
	EncodeColor(&vx[1], Color);

	vx[2].Position = D3DXVECTOR3(1,-1,-1);
	EncodeColor(&vx[2], Color);
	vx[3].Position = D3DXVECTOR3(1,-1,1);
	EncodeColor(&vx[3], Color);

	vx[4].Position = D3DXVECTOR3(1,-1,1);
	EncodeColor(&vx[4], Color);
	vx[5].Position = D3DXVECTOR3(-1,-1,1);
	EncodeColor(&vx[5], Color);

	vx[6].Position = D3DXVECTOR3(-1,-1,1);
	EncodeColor(&vx[6], Color);
	vx[7].Position = D3DXVECTOR3(-1,-1,-1);
	EncodeColor(&vx[7], Color);

	// Sides | | | |

	vx[8].Position = D3DXVECTOR3(-1,-1,-1);
	EncodeColor(&vx[8], Color);
	vx[9].Position = D3DXVECTOR3(-1,1,-1);
	EncodeColor(&vx[9], Color);

	vx[10].Position = D3DXVECTOR3(1,-1,-1);
	EncodeColor(&vx[10], Color);
	vx[11].Position = D3DXVECTOR3(1,1,-1);
	EncodeColor(&vx[11], Color);

	vx[12].Position = D3DXVECTOR3(1,-1,1);
	EncodeColor(&vx[12], Color);
	vx[13].Position = D3DXVECTOR3(1,1,1);
	EncodeColor(&vx[13], Color);

	vx[14].Position = D3DXVECTOR3(-1,-1,1);
	EncodeColor(&vx[14], Color);
	vx[15].Position = D3DXVECTOR3(-1,1,1);
	EncodeColor(&vx[15], Color);

	// Top
	vx[16].Position = D3DXVECTOR3(-1,1,-1);
	EncodeColor(&vx[16], Color);
	vx[17].Position = D3DXVECTOR3(1,1,-1);
	EncodeColor(&vx[17], Color);

	vx[18].Position = D3DXVECTOR3(1,1,-1);
	EncodeColor(&vx[18], Color);
	vx[19].Position = D3DXVECTOR3(1,1,1);
	EncodeColor(&vx[19], Color);

	vx[20].Position = D3DXVECTOR3(1,1,1);
	EncodeColor(&vx[20], Color);
	vx[21].Position = D3DXVECTOR3(-1,1,1);
	EncodeColor(&vx[21], Color);

	vx[22].Position = D3DXVECTOR3(-1,1,1);
	EncodeColor(&vx[22], Color);
	vx[23].Position = D3DXVECTOR3(-1,1,-1);
	EncodeColor(&vx[23], Color);

	HRESULT hr;
	LE(CreatePrimitive(vx, 24));

	return hr;
}

/** Creates a solid box */
HRESULT EditorLinePrimitive::CreateSolidBoxPrimitive(D3DXVECTOR4* Color, float Extends)
{
	LineVertex vx[36];
	int i=0;

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,-1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = D3DXVECTOR3(-1,1,1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	// Loop through all vertices and apply the extends
	for(i = 0; i < 36; i++)
	{
		*(D3DXVECTOR4 *)&vx[i].Position *= Extends;
	}

	HRESULT hr;
	LE(CreateSolidPrimitive(&vx[0], 36));

	return hr;
}

/** Creates the buffers and sets up the rest od the object */
HRESULT EditorLinePrimitive::CreateSolidPrimitive(LineVertex* PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
	HRESULT hr=S_OK;

	// Clean up previous data
	delete[] SolidVertices;
	if(SolidPrimVB)SolidPrimVB->Release();

	// Copy over the new data
	SolidVertices = new LineVertex[NumVertices];
	memcpy(SolidVertices, PrimVerts, sizeof(LineVertex)*NumVertices);
	this->NumSolidVertices = NumVertices;
	
	//Create the vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = NumVertices * sizeof(LineVertex);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &SolidVertices[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc,&InitData,&SolidPrimVB));

	SolidPrimitiveTopology = Topology;

	return hr;
}


/** Creates a plate, not of lines. Can't use intersection on this*/
HRESULT EditorLinePrimitive::CreateFilledCirclePrimitive(float Radius, UINT Detail, const D3DXVECTOR4* Color, int Axis)
{
	UINT NumVerts = Detail*3;
	LineVertex* vx = new LineVertex[NumVerts];

	float Step = (D3DX_PI*2)/((float)(Detail)-1);
	float s = 0;

	size_t i=0;
	while(i < NumVerts)
	{
		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = D3DXVECTOR3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), cosf(s), 0);
			break;
		}
		EncodeColor(&vx[i], Color);
		s+=Step;
		i++;


		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = D3DXVECTOR3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), cosf(s), 0);
			break;
		}
		EncodeColor(&vx[i], Color);
		//s+=Step;
		i++;

		vx[i].Position = D3DXVECTOR3(0, 0, 0);
		EncodeColor(&vx[i], Color);
		i++;

	}

	HRESULT hr = CreatePrimitive(vx, NumVerts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	delete[] vx;
	return hr;
}

/** Creates a circle of lines */
HRESULT EditorLinePrimitive::CreateCirclePrimitive(float Radius, UINT Detail, const D3DXVECTOR4* Color, int Axis)
{
	LineVertex* vx = new LineVertex[Detail];

	float Step = (D3DX_PI*2)/((float)Detail-1);
	float s = 0;

	for(UINT i = 0; i < Detail; i++)
	{
		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = D3DXVECTOR3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = D3DXVECTOR3(sinf(s), cosf(s), 0);
			break;
		}
		
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	HRESULT hr = CreatePrimitive(vx, Detail, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	delete[] vx;
	return hr;
}

/** Creates a ball of lines.*/
HRESULT EditorLinePrimitive::CreateLineBallPrimitive(UINT Detail, const D3DXVECTOR4* Color)
{
	// Allocate enough memory for all 3 slices
	LineVertex* vx = new LineVertex[(Detail*3)];

	float Step = (D3DX_PI*2)/((float)Detail-1);
	float s = 0;

	// Create first
	for(UINT i = 0; i < Detail; i++)
	{
		vx[i].Position = D3DXVECTOR3(sinf(s), 0, cosf(s));
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	s=0;

	// Create second
	for(UINT i = Detail; i < Detail*2; i++)
	{
		vx[i].Position = D3DXVECTOR3(0, sinf(s), cosf(s));
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	s=0;

	// Create third
	for(UINT i = Detail*2; i < Detail*3; i++)
	{
		vx[i].Position = D3DXVECTOR3(sinf(s), cosf(s), 0);
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	// Fix the last position
	//vx[(Detail*3)].Position = D3DXVECTOR3(sinf(s), cosf(s), 0);

	HRESULT hr = CreatePrimitive(vx, (Detail*3), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	delete[] vx;

	return hr;
}


/** Creates an arrow of lines */
HRESULT EditorLinePrimitive::CreateArrowPrimitive(const D3DXVECTOR4* Color,float ArrowRadius, float ArrowOffset)
{
	LineVertex vx[10];

	

	// Middle streak
	vx[0].Position = D3DXVECTOR3(0, 0, 0);
	vx[1].Position = D3DXVECTOR3(1, 0, 0);

	// Outer lines
	vx[2].Position = D3DXVECTOR3(1, 0, 0);
	vx[3].Position = D3DXVECTOR3(ArrowOffset, ArrowRadius, ArrowRadius);

	vx[4].Position = D3DXVECTOR3(1, 0, 0);
	vx[5].Position = D3DXVECTOR3(ArrowOffset, ArrowRadius, -ArrowRadius);

	vx[6].Position = D3DXVECTOR3(1, 0, 0);
	vx[7].Position = D3DXVECTOR3(ArrowOffset, -ArrowRadius, ArrowRadius);

	vx[8].Position = D3DXVECTOR3(1, 0, 0);
	vx[9].Position = D3DXVECTOR3(ArrowOffset, -ArrowRadius, -ArrowRadius);

	for(int i=0; i< 10; i++)
	{
		EncodeColor(&vx[i], Color);
	}
	HRESULT hr = CreatePrimitive(vx, 10);
	return hr;
}

/** Creates a cone of lines */
HRESULT EditorLinePrimitive::CreateSimpleConePrimitive(float Length, float Radius, UINT Detail, const D3DXVECTOR4* Color)
{
	UINT NumVerts;
	NumVerts = Detail*2; // Two for each connection line
	NumVerts += Detail*2; // Two for each circle line

	LineVertex* vx = new LineVertex[NumVerts];

	float Step = (D3DX_PI*2)/((float)Detail-1);
	float s = 0;

	UINT i = 0;
	while(i < NumVerts)
	{
		// First vertex of the circle-line
		vx[i].Position = D3DXVECTOR3(Length, (sinf(s)*Radius), cosf(s)*Radius);	
		EncodeColor(&vx[i], Color);
		i++;

		s+=Step;

		// Second vertex of the circle-line
		vx[i].Position = D3DXVECTOR3(Length, (sinf(s)*Radius), cosf(s)*Radius);	
		EncodeColor(&vx[i], Color);

		i++;
		
		// Connector line #1
		vx[i].Position = vx[i-2].Position;
		EncodeColor(&vx[i], Color);
		i++;

		// Connector line #2
		vx[i].Position = D3DXVECTOR3(0, 0, 0);
		EncodeColor(&vx[i], Color);
		i++;

		

		
	}

	HRESULT hr = CreatePrimitive(vx, NumVerts);

	delete[] vx;

	return hr;
}

/** Puts the color into the Normal and TexCoord channels */
void EditorLinePrimitive::EncodeColor(LineVertex* vx,const D3DXVECTOR4* Color)
{
	vx->Color = *Color;
}


/** Intersects the whole primitive. If hit, it returns a distance other than -1 */
float EditorLinePrimitive::IntersectPrimitive(D3DXVECTOR3* RayOrigin, D3DXVECTOR3* RayDirection, float Epsilon)
{
	UINT i;
	float Shortest = -1;

	// Bring the ray into object space
	D3DXMATRIX invWorld;
	D3DXMatrixInverse(&invWorld, NULL, &WorldMatrix);

	D3DXVECTOR3 Origin;
	D3DXVECTOR3 Dir;
	D3DXVec3TransformCoord(&Origin, RayOrigin, &invWorld);
	D3DXVec3TransformNormal(&Dir, RayDirection, &invWorld);

	// Go through all line segments and intersect them
	for(i = 0; i < NumVertices; i+=2)
	{	
		D3DXVECTOR3 vx[2];
		D3DXVec3TransformCoord(&vx[0], Vertices[i].Position.toD3DXVECTOR3(), &WorldMatrix);
		D3DXVec3TransformCoord(&vx[1], Vertices[i+1].Position.toD3DXVECTOR3(), &WorldMatrix);

		//float Dist = IntersectLineSegment(&Origin, &Dir, Vertices[i].Position.toD3DXVECTOR3(), Vertices[i+1].Position.toD3DXVECTOR3(), Epsilon);
		float Dist = IntersectLineSegment(RayOrigin, RayDirection, &vx[0], &vx[1], Epsilon);
		if(Dist < Shortest || Shortest == -1)
		{
			Shortest = Dist / Scale.x;
		}
	}

	if(NumSolidVertices>0)
	{
		FLOAT fBary1, fBary2;
		FLOAT fDist;
		int NumIntersections=0;
		Shortest = FLT_MAX;
		
		for( DWORD i = 0; i < NumSolidVertices; i+=3 )
		{
			D3DXVECTOR3 v0 = *SolidVertices[i + 0].Position.toD3DXVECTOR3();
			D3DXVECTOR3 v1 = *SolidVertices[i + 1].Position.toD3DXVECTOR3();
			D3DXVECTOR3 v2 = *SolidVertices[i + 2].Position.toD3DXVECTOR3();

			// Check if the pick ray passes through this point
			if( IntersectTriangle( &Origin, &Dir, v0, v1, v2,
				&fDist, &fBary1, &fBary2 ) )
			{
				if( fDist < Shortest  || Shortest == -1)
				{
					NumIntersections++;
					Shortest=0;
								
					//if( NumIntersections == MAX_INTERSECTIONS )
					//	break;
				}
			}
		}
	}

	return Shortest;
}

bool EditorLinePrimitive::IntersectTriangle( const D3DXVECTOR3* orig, const D3DXVECTOR3* dir,
                        D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                        FLOAT* t, FLOAT* u, FLOAT* v )
{
    // Find vectors for two edges sharing vert0
    D3DXVECTOR3 edge1 = v1 - v0;
    D3DXVECTOR3 edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate U parameter
    D3DXVECTOR3 pvec;
    D3DXVec3Cross( &pvec, dir, &edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    FLOAT det = D3DXVec3Dot( &edge1, &pvec );

    D3DXVECTOR3 tvec;
    if( det > 0 )
    {
        tvec = (*orig) - v0;
    }
    else
    {
        tvec = v0 - (*orig);
        det = -det;
    }

    if( det < 0.0001f )
        return FALSE;

    // Calculate U parameter and test bounds
    *u = D3DXVec3Dot( &tvec, &pvec );
    if( *u < 0.0f || *u > det )
        return FALSE;

    // Prepare to test V parameter
    D3DXVECTOR3 qvec;
    D3DXVec3Cross( &qvec, &tvec, &edge1 );

    // Calculate V parameter and test bounds
    *v = D3DXVec3Dot( dir, &qvec );
    if( *v < 0.0f || *u + *v > det )
        return FALSE;

    // Calculate t, scale parameters, ray intersects triangle
    *t = D3DXVec3Dot( &edge2, &qvec );
    FLOAT fInvDet = 1.0f / det;
    *t *= fInvDet;
    *u *= fInvDet;
    *v *= fInvDet;

    return TRUE;
}


float EditorLinePrimitive::IntersectLineSegment(const D3DXVECTOR3* rayOrigin,const D3DXVECTOR3* rayVec,const D3DXVECTOR3* lineStart,const D3DXVECTOR3* lineEnd, float Epsilon)
{
	
	D3DXVECTOR3 u = *rayVec;
	D3DXVECTOR3 v = (*lineEnd) - (*lineStart);
	D3DXVECTOR3 w = (*rayOrigin) - (*lineStart);

	
	float a = D3DXVec3Dot(&u, &u);	// always >= 0
	float b = D3DXVec3Dot(&u, &v);
	float c = D3DXVec3Dot(&v, &v);	// always >= 0
	float d = D3DXVec3Dot(&u, &w);
	float e = D3DXVec3Dot(&v, &w);
	float D = a*c - b*b;	// always >= 0
	float sc, sN, sD = D;	// sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D;	// tc = tN / tD, default tD = D >= 0


	// compute the line parameters of the two closest points
	if (D < Epsilon) {	// the lines are almost parallel
		sN = 0.0;			// force using point P0 on segment S1
		sD = 1.0;			// to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else {				// get the closest points on the infinite lines
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) {	// sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
	}

	if (tN < 0.0) {		// tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {	  // tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) < Epsilon ? 0.0f : sN / sD);
	tc = (abs(tN) < Epsilon ? 0.0f : tN / tD);

	// get the difference of the two closest points
	D3DXVECTOR3 dP = w + (sc * u) - (tc * v);	// = S1(sc) - S2(tc)
	//info.iFracRay = sc;
	//info.iFracLine = tc;
	return D3DXVec3Length(&dP);	// return the closest distance
}

void EditorLinePrimitive::SetWorldMatrix(const D3DXMATRIX* World,const D3DXVECTOR3* Loc,const D3DXVECTOR3* Rot,const D3DXVECTOR3* Scale)
{
	Location=*Loc;
	Rotation=*Rot;
	this->Scale=*Scale;
	WorldMatrix = *World;
}

void EditorLinePrimitive::SetLocation(const D3DXVECTOR3& NewLoc)
{
	Location=NewLoc;
	RecalcTransforms();
}

void EditorLinePrimitive::SetRotation(const D3DXVECTOR3& NewRotation)
{
	Rotation=NewRotation;
	RecalcTransforms();
}

void EditorLinePrimitive::SetScale(const D3DXVECTOR3& NewScale)
{
	Scale=NewScale;
	RecalcTransforms();
}

void EditorLinePrimitive::RecalcTransforms()
{
	//Matrices we need
	D3DXMATRIX matWorld,matScale,MatRot,MatTemp;

	//Temporary translation
	D3DXVECTOR3 Trans;


	//Copy from the original location, 
	//so we can modify it without hurting anything
	Trans=Location;

	//Devide Trans through Scale
	/*Trans.x/=Scale.x;
	Trans.y/=Scale.y;
	Trans.z/=Scale.z;*/


	//Apply translation to the WorldMatrix
	D3DXMatrixTranslation(&matWorld,Trans.x,Trans.y,Trans.z);

	//Now scale another matrix
	D3DXMatrixScaling( &matScale, Scale.x, Scale.y, Scale.z );

	


	//Apply rotation
	D3DXMatrixIdentity(&MatRot);

	D3DXVECTOR3 DeltaRot = Rotation - RotationMatrixAngles;

	if(Rotation != D3DXVECTOR3(0,0,0))
	{
		// Calculate matrix with the new angles
		if(bLocalRotation)
		{
			D3DXVECTOR3 Up(0,1,0);
			D3DXVECTOR3 Front(1,0,0);
			D3DXVECTOR3 Right;
			

			D3DXVec3TransformNormal(&Up, &Up, &RotationMatrix);
			D3DXVec3TransformNormal(&Front, &Front, &RotationMatrix);
			D3DXVec3Cross(&Right, &Up, &Front);

			D3DXMATRIX X;
			D3DXMatrixRotationAxis(&X, &Front, DeltaRot.x);

			D3DXMATRIX Y;
			D3DXMatrixRotationAxis(&Y, &Up, DeltaRot.y);

			D3DXMATRIX Z;
			D3DXMatrixRotationAxis(&Z, &Right, DeltaRot.z);

			RotationMatrix *= X * Y * Z;
		}else
		{
			D3DXMatrixIdentity(&MatRot);

			D3DXMatrixRotationAxis(&MatTemp, &D3DXVECTOR3(1,0,0), Rotation.x);        // Pitch
			D3DXMatrixMultiply(&MatRot, &MatRot, &MatTemp);
			D3DXMatrixRotationAxis(&MatTemp, &D3DXVECTOR3(0,1,0), Rotation.y);         // Yaw
			D3DXMatrixMultiply(&MatRot, &MatRot, &MatTemp);
			D3DXMatrixRotationAxis(&MatTemp, &D3DXVECTOR3(0,0,1), Rotation.z);       // Roll
			D3DXMatrixMultiply(&RotationMatrix, &MatRot, &MatTemp);

			//RotationMatrix = X * Y * Z;
		}

		RotationMatrixAngles = Rotation;
	}else if(!bJustUseRotationMatrix)
	{
		// Reset matrix to identity (Todo: ROTATION! Ò.ó Y U NO WORK!? (As I want))
		D3DXMatrixIdentity(&RotationMatrix);
		RotationMatrixAngles = D3DXVECTOR3(0,0,0);
	}


	WorldMatrix = matScale * RotationMatrix * matWorld;
	
}

/** Creates the buffers and sets up the rest od the object */
HRESULT EditorLinePrimitive::CreatePrimitive(LineVertex* PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
	HRESULT hr=S_OK;

	// Clean up previous data
	DeleteContent();

	// Copy over the new data
	Vertices = new LineVertex[NumVertices];
	memcpy(Vertices, PrimVerts, sizeof(LineVertex)*NumVertices);
	this->NumVertices = NumVertices;
	
	//Create the vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = NumVertices * sizeof(LineVertex);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &Vertices[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc,&InitData,&PrimVB));

	PrimitiveTopology = Topology;

	return hr;
}

/** Sets the shader to render with */
void EditorLinePrimitive::SetShader(D3D11PShader* Shader)
{
	PrimShader = Shader;
}

/** Sets the solid shader to render with */
void EditorLinePrimitive::SetSolidShader(D3D11PShader* SolidShader)
{
	SolidPrimShader = SolidShader;
}

/** Renders a vertexbuffer with the given shader */
void EditorLinePrimitive::RenderVertexBuffer(ID3D11Buffer* VB, UINT NumVertices, D3D11PShader* Shader, D3D11_PRIMITIVE_TOPOLOGY Topology, int Pass)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	D3DXMATRIX tr; D3DXMatrixTranspose(&tr, &WorldMatrix);
	Engine::GAPI->SetWorldTransform(tr);

	engine->SetActiveVertexShader("VS_Lines");
	engine->SetActivePixelShader("PS_Lines");
	
	engine->SetupVS_ExMeshDrawCall();
	engine->SetupVS_ExConstantBuffer();
	engine->SetupVS_ExPerInstanceConstantBuffer();

	engine->SetDefaultStates();
	Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	engine->UpdateRenderStates();

	Shader->Apply();

	// Set vertex buffer
	UINT stride = sizeof( LineVertex);
	UINT offset = 0;
	engine->GetContext()->IASetVertexBuffers( 0, 1, &VB, &stride, &offset );
	engine->GetContext()->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, NULL );
	
	engine->GetContext()->IASetPrimitiveTopology( Topology );

	engine->GetContext()->Draw(NumVertices, 0);
}

HRESULT EditorLinePrimitive::RenderPrimitive(int Pass)
{
	if(!PrimShader && !SolidPrimShader)
	{
		return E_FAIL;
	}

	if(bHidden)
	{
		return S_OK;
	}

	if(NumVertices > 0 && PrimShader)
	{
		RenderVertexBuffer(PrimVB, NumVertices, PrimShader, PrimitiveTopology, Pass);
	}

	if(NumSolidVertices > 0 && SolidPrimShader)
	{
		RenderVertexBuffer(SolidPrimVB, NumSolidVertices, SolidPrimShader, SolidPrimitiveTopology, Pass);
	}
	
	return S_OK;
}
