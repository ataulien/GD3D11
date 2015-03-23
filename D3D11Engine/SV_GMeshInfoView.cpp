#include "pch.h"
#include "SV_GMeshInfoView.h"
#include "D3D11GraphicsEngine.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "RenderToTextureBuffer.h"
#include "zCTexture.h"
#include "SV_Panel.h"
#include "D2DView.h"
#include "D3D11Texture.h"

const float MESH_ROT_SPEED = 0.01f;
const float ZOOM_SPEED = 0.01f;
const float DEFAULT_FOV = 90.0f;

SV_GMeshInfoView::SV_GMeshInfoView(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	Panel = new SV_Panel(view, this);
	//Panel->SetRect(D2D1::RectF(0, SV_TABCONTROL_HEADER_SIZE_Y, GetSize().width, GetSize().height));
	Panel->SetRenderMode(SV_Panel::PR_Image);
	Panel->SetGlossyOutline(true);

	IsDraggingView = false;

	ObjectPosition = D3DXVECTOR3(0,0,0);
	SetObjectOrientation(0, 0, 10.0f);
	RT = NULL;
	DS = NULL;
	VisualInfo = NULL;
	RenderMode = RM_Lit;

	FOV = DEFAULT_FOV;
}


SV_GMeshInfoView::~SV_GMeshInfoView(void)
{
	delete RT;
	delete DS;
}

/** Sets the current render mode */
void SV_GMeshInfoView::SetRenderMode(ERenderMode mode)
{
	RenderMode = mode;
}

/** Sets the mesh infos for this view */
void SV_GMeshInfoView::SetMeshes(const std::map<zCTexture*, MeshInfo*>& meshes, BaseVisualInfo* visInfo)
{
	Meshes = meshes;
	VisualInfo = visInfo;
	FOV = DEFAULT_FOV;

	// Find boundingbox of skeletal
	D3DXVECTOR3 bbmin = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
	D3DXVECTOR3 bbmax = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for(auto it = Meshes.begin();it != Meshes.end();it++)
	{
		std::vector<ExVertexStruct>& vertices = (*it).second->Vertices;
		for(unsigned int i=0;i<vertices.size();i++)
		{
			// Check bounding box
			bbmin.x = bbmin.x > vertices[i].Position.x ? vertices[i].Position.x : bbmin.x;
			bbmin.y = bbmin.y > vertices[i].Position.y ? vertices[i].Position.y : bbmin.y;
			bbmin.z = bbmin.z > vertices[i].Position.z ? vertices[i].Position.z : bbmin.z;
								
			bbmax.x = bbmax.x < vertices[i].Position.x ? vertices[i].Position.x : bbmax.x;
			bbmax.y = bbmax.y < vertices[i].Position.y ? vertices[i].Position.y : bbmax.y;
			bbmax.z = bbmax.z < vertices[i].Position.z ? vertices[i].Position.z : bbmax.z;
		}
	}

	ObjectPosition = -(bbmin * 0.5f + bbmax * 0.5f);
	SetObjectOrientation(0, 0, D3DXVec3Length(&(bbmin - bbmax)) / 2.0f);
}


/** Sets the name of this view */
void SV_GMeshInfoView::SetName(const std::string& name)
{

}

/** Sets the rotation of this object in the view */
void SV_GMeshInfoView::SetObjectOrientation(float yaw, float pitch, float distance)
{
	ObjectYaw = yaw;
	ObjectPitch = pitch;
	ObjectDistance = distance;

	D3DXMatrixTranslation(&ObjectWorldMatrix, ObjectPosition.x, ObjectPosition.y, ObjectPosition.z);

	D3DXMATRIX rotY;
	D3DXMATRIX rotZ;
	D3DXMatrixRotationY(&rotY, yaw);
	D3DXMatrixRotationZ(&rotZ, pitch);
	ObjectWorldMatrix *= rotY * rotZ;

	D3DXMatrixTranspose(&ObjectWorldMatrix, &ObjectWorldMatrix);

	D3DXMatrixLookAtLH(&ObjectViewMatrix, &D3DXVECTOR3(-distance,0,0), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));
	D3DXMatrixTranspose(&ObjectViewMatrix, &ObjectViewMatrix);
}

/** Updates the view */
void SV_GMeshInfoView::UpdateView()
{
	if(IsHidden())
		return;

	D3D11GraphicsEngine* g = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	D3DXMatrixPerspectiveFovLH(&ObjectProjMatrix, (float)D3DXToRadian(FOV), GetSize().height / GetSize().width, 0.01f, 10000.0f);
	D3DXMatrixTranspose(&ObjectProjMatrix, &ObjectProjMatrix);

	g->SetDefaultStates();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	D3DXMATRIX oldProj = Engine::GAPI->GetProjTransform();

	// Set transforms
	Engine::GAPI->SetWorldTransform(ObjectWorldMatrix);
	Engine::GAPI->SetViewTransform(ObjectViewMatrix);
	Engine::GAPI->SetProjTransform(ObjectProjMatrix);

	// Set Viewport
	D3D11_VIEWPORT oldVP; 
	UINT numVP = 1;
	g->GetContext()->RSGetViewports(&numVP, &oldVP);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = GetSize().width;
	vp.Height = GetSize().height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	g->GetContext()->RSSetViewports(1, &vp);

	// Clear
	g->GetContext()->ClearRenderTargetView(RT->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	g->GetContext()->ClearDepthStencilView(DS->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0); 

	// Bind RTV
	g->GetContext()->OMSetRenderTargets(1, RT->GetRenderTargetViewPtr(), DS->GetDepthStencilView());

	// Setup shaders
	g->SetActiveVertexShader("VS_Ex");
	g->SetActivePixelShader("PS_DiffuseAlphaTest");

	switch(RenderMode)
	{
	case RM_Lit:
		g->SetActivePixelShader("PS_Preview_TexturedLit");
		DrawMeshes();
		break;

	case RM_Textured:
		g->SetActivePixelShader("PS_Preview_Textured");
		DrawMeshes();
		break;

	case RM_TexturedWireFrame:
		g->SetActivePixelShader("PS_Preview_Textured");
		DrawMeshes();
		// No break here, render wireframe right after

	case RM_Wireframe:
		g->SetActivePixelShader("PS_Preview_White");
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
		DrawMeshes();
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
		break;
	}
	// Reset viewport
	g->GetContext()->RSSetViewports(1, &oldVP);
	Engine::GAPI->SetProjTransform(oldProj);

	// Update panel
	Panel->SetD3D11TextureAsImage(RT->GetTexture(), INT2(RT->GetSizeX(), RT->GetSizeY()));
}

/** Draws the meshes to the buffer */
void SV_GMeshInfoView::DrawMeshes()
{
	D3D11GraphicsEngine* g = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	g->SetupVS_ExMeshDrawCall();
	g->SetupVS_ExConstantBuffer();
	g->SetupVS_ExPerInstanceConstantBuffer();

	VisualTesselationSettings* ts = NULL;
	if(VisualInfo)
		ts = &VisualInfo->TesselationInfo;

	// Draw each texture
	for(auto it = Meshes.begin(); it != Meshes.end(); it++)
	{
		// Set up tesselation if wanted
		if(ts && !(*it).second->IndicesPNAEN.empty() && ts->buffer.VT_TesselationFactor > 0.0f)
		{
			g->Setup_PNAEN(D3D11GraphicsEngine::PNAEN_Default);
			ts->Constantbuffer->BindToDomainShader(1);
			ts->Constantbuffer->BindToHullShader(1);

			

			if((*it).first->CacheIn(-1) == zRES_CACHED_IN)
			{
				MyDirectDrawSurface7* surface = (*it).first->GetSurface();
				ID3D11ShaderResourceView* srv = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;

				// Draw
				(*it).first->Bind(0);
				g->DrawVertexBufferIndexed( (*it).second->MeshVertexBuffer,
											(*it).second->MeshIndexBufferPNAEN,
											(*it).second->IndicesPNAEN.size() );
			}
		}else if(VisualInfo)
		{
			g->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g->GetContext()->DSSetShader(NULL, NULL, NULL);
			g->GetContext()->HSSetShader(NULL, NULL, NULL);
			g->SetActiveHDShader("");
			g->SetActiveVertexShader("VS_Ex");

			if((*it).first && (*it).first->CacheIn(-1) == zRES_CACHED_IN)
			{
				// Draw
				(*it).first->Bind(0);
				g->DrawVertexBufferIndexed( (*it).second->MeshVertexBuffer,
											(*it).second->MeshIndexBuffer,
											(*it).second->Indices.size() );
			}
		}


		
	}

	
}

/** Sets the position and size of this sub-view */
void SV_GMeshInfoView::SetRect(const D2D1_RECT_F& rect)
{
	D3D11GraphicsEngine* g = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	D2DSubView::SetRect(rect);

	Panel->SetRect(D2D1::RectF(0, 0, GetSize().width, GetSize().height));

	// Create new RT
	delete RT;
	RT = new RenderToTextureBuffer(g->GetDevice(), 
		(UINT)std::max(8.0f, GetSize().width), 
		(UINT)std::max(8.0f, GetSize().height), 
		DXGI_FORMAT_R8G8B8A8_UNORM);

	delete DS;
	DS = new RenderToDepthStencilBuffer(g->GetDevice(), 
		(UINT)std::max(8.0f, GetSize().width), 
		(UINT)std::max(8.0f, GetSize().height), 
		DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_GMeshInfoView::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{
	case WM_MOUSEMOVE:
		if(IsDraggingView)
		{
			POINT p = D2DView::GetCursorPosition();

			float yaw, pitch, distance;
			GetObjectOrientation(yaw, pitch, distance);

			yaw += (LastDragPosition.x - p.x) * MESH_ROT_SPEED;
			pitch -= (LastDragPosition.y - p.y) * MESH_ROT_SPEED;

			SetObjectOrientation(yaw, pitch, distance);
			UpdateView();

			LastDragPosition = p;
			return false;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();

			D2D1_RECT_F panelrect = D2D1::RectF(Panel->GetPosition().x + clientRectAbs.left, 
				Panel->GetPosition().y + clientRectAbs.top, 
				Panel->GetSize().width + clientRectAbs.left, Panel->GetSize().height + clientRectAbs.top);
			if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), panelrect))
			{
				IsDraggingView = true;
				LastDragPosition = p;
				return false;
			}
		}
		break;

	case WM_LBUTTONUP:
		if(IsDraggingView)
		{
			IsDraggingView = false;
			return false;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			POINT p = D2DView::GetCursorPosition();

				D2D1_RECT_F panelrect = D2D1::RectF(Panel->GetPosition().x + clientRectAbs.left, 
					Panel->GetPosition().y + clientRectAbs.top, 
					Panel->GetSize().width + clientRectAbs.left, Panel->GetSize().height + clientRectAbs.top);
				if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), panelrect))
				{
					FOV += GET_WHEEL_DELTA_WPARAM(wParam) * ZOOM_SPEED;
					UpdateView();
				}
		}
		break;
	}
	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}

/** Returns the view-properties */
void SV_GMeshInfoView::GetObjectOrientation(float& yaw, float& pitch, float& distance)
{
	yaw = ObjectYaw;
	pitch = ObjectPitch;
	distance = ObjectDistance;
}