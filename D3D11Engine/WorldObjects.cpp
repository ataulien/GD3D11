#include "pch.h"
#include "WorldObjects.h"
#include "GothicAPI.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "zCVob.h"
#include "zCMaterial.h"
#include "zCTexture.h"

const int WORLDMESHINFO_VERSION = 5;
const int VISUALINFO_VERSION = 5;

/** Saves the info for this visual */
void WorldMeshInfo::SaveWorldMeshInfo(const std::string& name)
{
	FILE* f = fopen(("system\\GD3D11\\meshes\\infos\\" + name + ".wi").c_str(), "wb");

	if(!f)
	{
		LogError() << "Failed to open file '" << ("system\\GD3D11\\meshes\\infos\\" + name + ".wi") << "' for writing! Make sure the game runs in Admin mode "
					  "to get the rights to write to that directory!";

		return;
	}

	// Write the version first
	fwrite(&WORLDMESHINFO_VERSION, sizeof(WORLDMESHINFO_VERSION), 1, f);

	// Then the TesselationInfo
	fwrite(&TesselationSettings.buffer, sizeof(TesselationSettings), 1, f);
	Toolbox::SaveStringToFILE(f, TesselationSettings.TesselationShader);

	fclose(f);
}

/** Loads the info for this visual */
void WorldMeshInfo::LoadWorldMeshInfo(const std::string& name)
{
	FILE* f = fopen(("system\\GD3D11\\meshes\\infos\\" + name + ".wi").c_str(), "rb");

	if(!f)
	{
		// Silently fail here, since it is totally valid for a visual to not have an info-file
		return;
	}

	// Read the version first
	int version;
	fread(&version, sizeof(version), 1, f);

	// Then the TesselationInfo
	fread(&TesselationSettings.buffer, sizeof(TesselationSettings), 1, f);
	TesselationSettings.TesselationShader = Toolbox::LoadStringFromFILE(f);

	TesselationSettings.UpdateConstantbuffer();

	// Create actual PNAEN-Info if needed
	if(TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
	{
		WorldConverter::CreatePNAENInfoFor(this, TesselationSettings.buffer.VT_DisplacementStrength > 0.0f);
	}

	fclose(f);
}

/** Updates the vobs constantbuffer */
void VobInfo::UpdateVobConstantBuffer()
{
	VS_ExConstantBuffer_PerInstance cb;
	cb.World = *Vob->GetWorldMatrixPtr();

	VobConstantBuffer->UpdateBuffer(&cb);

	LastRenderPosition = Vob->GetPositionWorld();
	WorldMatrix = cb.World;

	// Colorize the vob according to the underlaying polygon
	if(IsIndoorVob)
	{
		// All lightmapped polys have this color, so just use it
		GroundColor = DEFAULT_LIGHTMAP_POLY_COLOR;
	}else
	{
		// Get the color of the first found feature of the ground poly
		GroundColor = Vob->GetGroundPoly() ? Vob->GetGroundPoly()->getFeatures()[0]->lightStatic : 0xFFFFFFFF;
	}

	//D3DXMatrixTranspose(&WorldMatrix, &cb.World);
}

/** Updates the vobs constantbuffer */
void SkeletalVobInfo::UpdateVobConstantBuffer()
{
	VS_ExConstantBuffer_PerInstance cb;
	cb.World = *Vob->GetWorldMatrixPtr();

	WorldMatrix = cb.World;

	if(!VobConstantBuffer)
		Engine::GraphicsEngine->CreateConstantBuffer(&VobConstantBuffer, &cb, sizeof(cb));
	else
		VobConstantBuffer->UpdateBuffer(&cb);
}

/** creates/updates the constantbuffer */
void VisualTesselationSettings::UpdateConstantbuffer()
{
	if(Constantbuffer)
	{
		Constantbuffer->UpdateBuffer(&buffer);
	}else
	{
		Engine::GraphicsEngine->CreateConstantBuffer(&Constantbuffer, &buffer, sizeof(buffer));
	}
}


/** Creates PNAEN-Info for all meshes if not already there */
void SkeletalMeshVisualInfo::CreatePNAENInfo(bool softNormals)
{
	for(std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>>::iterator it = SkeletalMeshes.begin(); it != SkeletalMeshes.end(); it++)
	{
		for(unsigned int i=0;i<(*it).second.size();i++)
		{
			if((*it).second[i]->IndicesPNAEN.empty())
				WorldConverter::CreatePNAENInfoFor((*it).second[i], Meshes[(*it).first][i], softNormals);
		}
	}
}

/** Removes PNAEN info from this visual */
void SkeletalMeshVisualInfo::ClearPNAENInfo()
{
	for(std::map<zCMaterial *, std::vector<SkeletalMeshInfo*>>::iterator it = SkeletalMeshes.begin(); it != SkeletalMeshes.end(); it++)
	{
		for(unsigned int i=0;i<(*it).second.size();i++)
		{
			delete (*it).second[i]->MeshIndexBufferPNAEN;
			(*it).second[i]->MeshIndexBufferPNAEN = NULL;

			(*it).second[i]->IndicesPNAEN.clear();	
		}
	}

	BaseVisualInfo::ClearPNAENInfo();
}

/** Creates PNAEN-Info for all meshes if not already there */
void MeshVisualInfo::CreatePNAENInfo(bool softNormals)
{
	for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
	{
		for(unsigned int i=0;i<(*it).second.size();i++)
		{
			if((*it).second[i]->IndicesPNAEN.empty())
				WorldConverter::CreatePNAENInfoFor((*it).second[i], softNormals);
		}
	}
}

/** Removes PNAEN info from this visual */
void BaseVisualInfo::ClearPNAENInfo()
{
	for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
	{
		for(unsigned int i=0;i<(*it).second.size();i++)
		{
			delete (*it).second[i]->MeshIndexBufferPNAEN;
			(*it).second[i]->MeshIndexBufferPNAEN = NULL;

			(*it).second[i]->VerticesPNAEN.clear();
			(*it).second[i]->IndicesPNAEN.clear();	
		}
	}
}

/** Saves the info for this visual */
void BaseVisualInfo::SaveMeshVisualInfo(const std::string& name)
{
	FILE* f = fopen(("system\\GD3D11\\meshes\\infos\\" + name + ".vi").c_str(), "wb");

	if(!f)
	{
		LogError() << "Failed to open file '" << ("system\\GD3D11\\meshes\\infos\\" + name + ".vi") << "' for writing! Make sure the game runs in Admin mode "
					  "to get the rights to write to that directory!";

		return;
	}

	// Write the version first
	fwrite(&VISUALINFO_VERSION, sizeof(VISUALINFO_VERSION), 1, f);

	// Then the TesselationInfo
	fwrite(&TesselationInfo.buffer, sizeof(TesselationInfo), 1, f);
	Toolbox::SaveStringToFILE(f, TesselationInfo.TesselationShader);

	fclose(f);
}

/** Loads the info for this visual */
void BaseVisualInfo::LoadMeshVisualInfo(const std::string& name)
{
	FILE* f = fopen(("system\\GD3D11\\meshes\\infos\\" + name + ".vi").c_str(), "rb");

	if(!f)
	{
		// Silently fail here, since it is totally valid for a visual to not have an info-file
		return;
	}

	// Read the version first
	int version;
	fread(&version, sizeof(version), 1, f);

	// Then the TesselationInfo
	fread(&TesselationInfo.buffer, sizeof(TesselationInfo), 1, f);
	TesselationInfo.TesselationShader = Toolbox::LoadStringFromFILE(f);

	TesselationInfo.UpdateConstantbuffer();

	fclose(f);
}

SectionInstanceCache::~SectionInstanceCache()
{
	for(std::map<MeshVisualInfo *, D3D11VertexBuffer*>::iterator it = InstanceCache.begin(); it != InstanceCache.end(); it++)
	{
		delete (*it).second;
	}
}

MeshInfo::~MeshInfo()
{
	//Engine::GAPI->GetRendererState()->RendererInfo.VOBVerticesDataSize -= Indices.size() * sizeof(VERTEX_INDEX);
	//Engine::GAPI->GetRendererState()->RendererInfo.VOBVerticesDataSize -= Vertices.size() * sizeof(ExVertexStruct);

	delete MeshVertexBuffer;
	delete MeshIndexBuffer;
	delete MeshIndexBufferPNAEN;

}

SkeletalMeshInfo::~SkeletalMeshInfo()
{
	Engine::GAPI->GetRendererState()->RendererInfo.SkeletalVerticesDataSize -= Indices.size() * sizeof(VERTEX_INDEX);
	Engine::GAPI->GetRendererState()->RendererInfo.SkeletalVerticesDataSize -= Vertices.size() * sizeof(ExSkelVertexStruct);

	delete MeshVertexBuffer;
	delete MeshIndexBuffer;
	delete MeshIndexBufferPNAEN;
}

/** Clears the cache for the given progmesh */
void SectionInstanceCache::ClearCacheForStatic(MeshVisualInfo* pm)
{
	if(InstanceCache.find(pm) != InstanceCache.end())
	{
		D3D11VertexBuffer* vb = InstanceCache[pm];
		delete vb;
		InstanceCache[pm] = NULL;
		InstanceCacheData[pm].clear();
	}
}

/** Saves this sections mesh to a file */
void WorldMeshSectionInfo::SaveSectionMeshToFile(const std::string& name)
{
	FILE* f;
	fopen_s(&f, name.c_str(), "wb");

	if(!f)
		return;


}


/** Saves the mesh infos for this section */
void WorldMeshSectionInfo::SaveMeshInfos(const std::string& worldName, INT2 sectionPos)
{
	for(auto it = WorldMeshes.begin(); it != WorldMeshes.end(); it++)
	{
		// TODO: Custom mesh!
		if((*it).first.Texture)
		{
			std::string tx = (*it).first.Texture->GetNameWithoutExt();
			std::string name = "WS_" + worldName + "_" + std::to_string(sectionPos.x) + "_" + std::to_string(sectionPos.y) + "_" + tx;

			if((*it).second->SaveInfo) /// Save only if marked dirty
				(*it).second->SaveWorldMeshInfo(name);
		}
	}
}

/** Saves the mesh infos for this section */
void WorldMeshSectionInfo::LoadMeshInfos(const std::string& worldName, INT2 sectionPos)
{
	for(auto it = WorldMeshes.begin(); it != WorldMeshes.end(); it++)
	{
		// TODO: Custom mesh!
		if((*it).first.Texture)
		{
			std::string tx = (*it).first.Texture->GetNameWithoutExt();
			std::string name = "WS_" + worldName + "_" + std::to_string(sectionPos.x) + "_" + std::to_string(sectionPos.y) + "_" + tx;

			(*it).second->LoadWorldMeshInfo(name);
		}
	}
}

/** Creates buffers for this mesh info */
XRESULT MeshInfo::Create(ExVertexStruct* vertices, unsigned int numVertices, VERTEX_INDEX* indices, unsigned int numIndices)
{
	Vertices.resize(numVertices);
	memcpy(&Vertices[0], vertices, numVertices * sizeof(ExVertexStruct));

	Indices.resize(numIndices);
	memcpy(&Indices[0], indices, numIndices * sizeof(VERTEX_INDEX));

	// Create the buffers
	Engine::GraphicsEngine->CreateVertexBuffer(&MeshVertexBuffer);
	Engine::GraphicsEngine->CreateVertexBuffer(&MeshIndexBuffer);

	// Init and fill it
	MeshVertexBuffer->Init(vertices, numVertices * sizeof(ExVertexStruct));
	MeshIndexBuffer->Init(indices, numIndices * sizeof(VERTEX_INDEX), D3D11VertexBuffer::B_INDEXBUFFER);

	Engine::GAPI->GetRendererState()->RendererInfo.VOBVerticesDataSize += numVertices * sizeof(ExVertexStruct);
	Engine::GAPI->GetRendererState()->RendererInfo.VOBVerticesDataSize += numIndices * sizeof(VERTEX_INDEX);

	return XR_SUCCESS;
}
