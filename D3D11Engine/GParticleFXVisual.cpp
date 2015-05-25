#include "pch.h"
#include "GParticleFXVisual.h"
#include "zCParticleFX.h"
#include "zCTexture.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"

GParticleFXVisual::GParticleFXVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{
}


GParticleFXVisual::~GParticleFXVisual(void)
{
}

/** Draws the visual for the given vob */
void GParticleFXVisual::DrawVisual(const RenderInfo& info)
{
	zCParticleFX* fx = (zCParticleFX *)SourceVisual;

#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	// Get our view-matrix
	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Update effects time
	fx->UpdateTime();

	// Maybe create more emitters?
	fx->CheckDependentEmitter();

	// Check if we even have to do something
	zTParticle* pfx = fx->GetFirstParticle();
	if(pfx)
	{
		// Get texture
		zCTexture* texture = NULL;
		if(fx->GetEmitter())
		{
			texture = fx->GetEmitter()->GetVisTexture();
			if(texture)
			{
				// Check if it's loaded
				if(texture->CacheIn(0.6f) != zRES_CACHED_IN)
					return;
			}else
			{
				return;
			}
		}


		// Check for kill (first particle)
		zTParticle*	kill = NULL;
		zTParticle*	p = NULL;

		while(true)
		{
			kill = pfx;
			if (kill && (kill->LifeSpan < *fx->GetPrivateTotalTime())) 
			{			
				//if (kill->PolyStrip)	
				//	kill->polyStrip->Release(); // TODO: MEMLEAK RIGHT HERE!

				pfx = kill->Next;
				fx->SetFirstParticle(pfx);

				kill->Next = *(zTParticle **)GothicMemoryLocations::GlobalObjects::s_globFreePart;
				*(zTParticle **)GothicMemoryLocations::GlobalObjects::s_globFreePart = kill;
				continue;
			}
			break;
		}

		int i=0;
		// Loop through all particles we have
		for (p = pfx ; p ; p=p->Next) 
		{
			// Run the strange particle-deletion-loop again
			while(true)
			{
				kill = p->Next;
				if (kill && (kill->LifeSpan < *fx->GetPrivateTotalTime() )) 
				{
					//if (kill->PolyStrip)	
					//	kill->polyStrip->Release(); // TODO: MEMLEAK RIGHT HERE!

					p->Next			= kill->Next;
					kill->Next = *(zTParticle **)GothicMemoryLocations::GlobalObjects::s_globFreePart;
					*(zTParticle **)GothicMemoryLocations::GlobalObjects::s_globFreePart = kill;
					continue;
				}
				break;
			}

			// Generate instance info
			ParticleInstanceInfo ii;
			ii.scale = D3DXVECTOR2(p->Size.x, p->Size.y);

			// Construct world matrix
			D3DXMATRIX world;
			int alignment = fx->GetEmitter()->GetVisAlignment();
			if(alignment == zPARTICLE_ALIGNMENT_XY)
			{
				D3DXMATRIX rot;
				D3DXMatrixRotationX(&rot, (float)(D3DX_PI / 2.0f));
				D3DXMatrixTranslation(&world, p->PositionWS.x, p->PositionWS.y, p->PositionWS.z);
				world = rot * world;
			}else
			{
				if(alignment == zPARTICLE_ALIGNMENT_VELOCITY)
				{
					D3DXMATRIX sw = *info.WorldMatrix;
					D3DXMatrixTranspose(&sw, &sw);

					D3DXVECTOR3 velNrm; D3DXVec3Normalize(&velNrm, &p->Vel);
					D3DXVec3Normalize(&velNrm, &velNrm);
					D3DXVECTOR3 velPosY; D3DXVec3TransformNormal(&velPosY, &velNrm, &sw);
					
					D3DXVECTOR3 velPosX = D3DXVECTOR3(-velPosY.y, velPosY.x, velPosY.z);
					D3DXVECTOR3 xdim = velPosX * ii.scale.x;
					D3DXVECTOR3 ydim = velPosY * ii.scale.y;

					ii.scale.x = -xdim.x + ydim.x;
					ii.scale.y = -xdim.y + ydim.y;

					if(abs(ii.scale.x) < abs(ii.scale.y))
						ii.scale.x = abs(ii.scale.y / 1.5f) * (ii.scale.x < 0 ? -1 : 1);
					else
						ii.scale.y = abs(ii.scale.x / 1.5f) * (ii.scale.y < 0 ? -1 : 1);
				}
				
				//D3DXMatrixIdentity(&world);
				D3DXMatrixTranslation(&world, p->PositionWS.x, p->PositionWS.y, p->PositionWS.z);
			}

			D3DXMatrixTranspose(&world, &world);

			D3DXMATRIX mat = view * world;
			
			// Undo all rotations from view
			if(alignment != zPARTICLE_ALIGNMENT_XY) 
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++) 
					{
						if ( x==y )
							mat(x,y) = 1.0;
						else
							mat(x,y) = 0.0;
					}
				}
			}

			// Put matrix into instance-info // TODO: Find a better way of this rather than 
											 // doing matrix calculations for every particle!
			ii.worldview = mat;

			// We only use quad polys, but the original engine has the option to
			// use a single triangle for a particle. To simulate this, we need to half the size
			// in case we have a triangle
			if(!fx->GetEmitter()->GetVisIsQuadPoly())
			{
				ii.scale.x *= 0.5f;
				ii.scale.y *= 0.5f;
			}

			// Get the color from the emitter
			float4 color;
			color.x = p->Color.x / 255.0f;
			color.y = p->Color.y / 255.0f;
			color.z = p->Color.z / 255.0f;

			// There are multiple alpha-handling modes
			if(fx->GetEmitter()->GetVisTexAniIsLooping() != 2) // 2 seems to be some magic case with sinus smoothing
			{
				color.w = std::min(p->Alpha, 255.0f) / 255.0f;
			}else
			{
				// Smooth alpha fade-in/out
				color.w = std::min((zCParticleFX::SinSmooth(fabs((p->Alpha - fx->GetEmitter()->GetVisAlphaStart()) * fx->GetEmitter()->GetAlphaDist())) * p->Alpha) / 255.0f, 255.0f);
			}

			// Don't allow alpha-values below 0 (Seems to happen sometimes..)
			color.w = std::max(color.w, 0.0f);

			// Put the color into the instance info
			ii.color = color.ToDWORD(); // FIXME: We don't really need floats here

			// Draw the particle
			DrawParticleInstance(info, ii, p->PositionWS);

			fx->UpdateParticle(p);
		}
	}

	// Create new particles?
	fx->CreateParticlesUpdateDependencies();

	// This causes an infinite loop on G1. TODO: Investigate!
#ifndef BUILD_GOTHIC_1_08k
	// Do something I dont exactly know what it does :)
	fx->GetStaticPFXList()->TouchPfx(fx);
#endif
}

/** Draws a single particle instance */
void GParticleFXVisual::DrawParticleInstance(const RenderInfo& info, const ParticleInstanceInfo& instance, const D3DXVECTOR3& positionWorld)
{
	Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(positionWorld, 25.0f, D3DXVECTOR4(1,0,1,1));
}