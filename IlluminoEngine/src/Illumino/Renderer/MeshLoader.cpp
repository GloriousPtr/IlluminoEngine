#include "ipch.h"
#include "MeshLoader.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <glm/glm.hpp>

namespace IlluminoEngine
{
	Ref<MeshBuffer> ProcessMesh(aiMesh *mesh, const aiScene *scene)
	{
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec2 UV;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex v;
			v.Position.x = mesh->mVertices[i].x;
			v.Position.y = mesh->mVertices[i].y;
			v.Position.z = mesh->mVertices[i].z;

			v.UV.x = mesh->mTextureCoords[0][i].x;
			v.UV.y = mesh->mTextureCoords[0][i].y;

			vertices.push_back(v);
		}

		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		return MeshBuffer::Create((float*)&vertices[0], &indices[0], vertices.size() * sizeof(Vertex), indices.size() * sizeof(uint32_t), sizeof(Vertex));
	}

	void ProcessNode(aiNode *node, const aiScene *scene, std::vector<Ref<MeshBuffer>>& meshes)
	{
		for (size_t i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}

		for (size_t i = 0; i < node->mNumChildren; ++i)
		{
			ProcessNode(node->mChildren[i], scene, meshes);
		}
	}

	void MeshLoader::LoadMesh(const char* filepath, std::vector<Ref<MeshBuffer>>& meshes)
	{
		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		const uint32_t flags = 
			aiProcess_MakeLeftHanded |									// Necessary for D3D applications
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
//			aiProcess_GlobalScale |
			aiProcess_ImproveCacheLocality |
			aiProcess_ValidateDataStructure;
		const aiScene* scene = importer.ReadFile(filepath, flags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			ILLUMINO_ERROR("Error occured while loading mesh: {0}", importer.GetErrorString());
			return;
		}
		
		ProcessNode(scene->mRootNode, scene, meshes);
	}
}
