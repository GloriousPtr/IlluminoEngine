#include "ipch.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include "Illumino/Utils/StringUtils.h"

namespace IlluminoEngine
{
	Mesh::Mesh(const char* filepath)
	{
		OPTICK_EVENT();

		Load(filepath);
	}

	void Mesh::Load(const char* filepath)
	{
		OPTICK_EVENT();

		Assimp::Importer importer;
		importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 80.0f);
		uint32_t meshImportFlags = 
			aiProcess_MakeLeftHanded |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_PreTransformVertices |
			aiProcess_SortByPType |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality |
			aiProcess_ValidateDataStructure;
		
		if (StringUtils::GetExtension(filepath) != "assbin")
		{
			meshImportFlags |=
				aiProcess_GenNormals |
				aiProcess_GenUVCoords |
				aiProcess_GlobalScale;
		}

		const aiScene *scene = importer.ReadFile(filepath, meshImportFlags);

		if (!scene)
		{
			ILLUMINO_ERROR("Could not import the file: {0}. Error: {1}", filepath, importer.GetErrorString());
			return;
		}

		ProcessNode(scene->mRootNode, scene, filepath);
		m_Name = StringUtils::GetName(filepath);
	}

	Submesh& Mesh::GetSubmesh(uint32_t index)
	{
		OPTICK_EVENT();

		ILLUMINO_ASSERT(index < m_Submeshes.size(), "Submesh index out of bounds");

		return m_Submeshes[index];
	}

	void Mesh::ProcessNode(aiNode *node, const aiScene *scene, const char* filepath)
	{
		OPTICK_EVENT();

		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			const char* nodeName = node->mName.C_Str();
			ProcessMesh(mesh, scene, filepath, nodeName);
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, filepath);
		}
	}

	eastl::vector<Ref<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* filepath)
	{
		OPTICK_EVENT();

		eastl::string path = eastl::string(filepath);
		eastl::string dir = path.substr(0, path.find_last_of('/'));
		eastl::vector<Ref<Texture2D>> textures;
		for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			eastl::string path = dir + '\\' + str.C_Str();
			Ref<Texture2D> texture = Texture2D::Create(path.c_str());
			textures.push_back(texture);
		}
		return textures;
	}

	void Mesh::ProcessMesh(aiMesh *mesh, const aiScene *scene, const char* filepath, const char* nodeName)
	{
		OPTICK_EVENT();

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec2 UV;
		};

		eastl::vector<Vertex> vertices;
		eastl::vector<uint32_t> indices;

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

		Ref<MeshBuffer> meshBuffer = MeshBuffer::Create((float*)&vertices[0], &indices[0], vertices.size() * sizeof(Vertex), indices.size() * sizeof(uint32_t), sizeof(Vertex));

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        eastl::vector<Ref<Texture2D>> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, filepath);
		//eastl::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		//eastl::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		//eastl::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		Ref<Texture2D> albedo = diffuseMaps.size() > 0 ? diffuseMaps[0] : nullptr;

		uint32_t index = m_Submeshes.size();
		m_Submeshes.push_back({ meshBuffer, albedo });
	}
}
