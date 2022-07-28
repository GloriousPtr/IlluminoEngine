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

		uint32_t meshImportFlags = aiProcess_MakeLeftHanded
			| aiProcess_FlipUVs;
		
		if (StringUtils::GetExtension(filepath) != "assbin")
		{
			meshImportFlags |=
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_PreTransformVertices |
				aiProcess_SortByPType |
				aiProcess_GenNormals |
				aiProcess_GenUVCoords |
				aiProcess_OptimizeMeshes |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GlobalScale |
				aiProcess_ImproveCacheLocality |
				aiProcess_ValidateDataStructure;
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
			ProcessMesh(mesh, scene, filepath, node->mName.C_Str());
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
		eastl::string dir = path.substr(0, path.find_last_of("\\"));
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
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Bitangent;
			glm::vec2 UV = glm::vec2(0.0);
		};

		eastl::vector<Vertex> vertices;
		eastl::vector<uint32_t> indices;

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex v;
			auto& vertexPos = mesh->mVertices[i];
			v.Position.x = vertexPos.x;
			v.Position.y = vertexPos.y;
			v.Position.z = vertexPos.z;

			auto& normal = mesh->mNormals[i];
			v.Normal.x = normal.x;
			v.Normal.y = normal.y;
			v.Normal.z = normal.z;

			if (mesh->mTangents)
			{
				auto& tangent = mesh->mTangents[i];
				v.Tangent.x = tangent.x;
				v.Tangent.y = tangent.y;
				v.Tangent.z = tangent.z;

				auto& bitangent = mesh->mBitangents[i];
				v.Bitangent.x = bitangent.x;
				v.Bitangent.y = bitangent.y;
				v.Bitangent.z = bitangent.z;
			}
			else
			{
				size_t index = i / 3;
				// Shortcuts for vertices
				auto& v0 = mesh->mVertices[index + 0];
				auto& v1 = mesh->mVertices[index + 1];
				auto& v2 = mesh->mVertices[index + 2];

				// Shortcuts for UVs
				auto& uv0 = mesh->mTextureCoords[0][index + 0];
				auto& uv1 = mesh->mTextureCoords[0][index + 1];
				auto& uv2 = mesh->mTextureCoords[0][index + 2];

				// Edges of the triangle : position delta
				auto deltaPos1 = v1 - v0;
				auto deltaPos2 = v2 - v0;

				// UV delta
				auto deltaUV1 = uv1 - uv0;
				auto deltaUV2 = uv2 - uv0;

				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				v.Tangent.x = tangent.x;
				v.Tangent.y = tangent.y;
				v.Tangent.z = tangent.z;

				v.Bitangent.x = bitangent.x;
				v.Bitangent.y = bitangent.y;
				v.Bitangent.z = bitangent.z;
			}

			if (mesh->mTextureCoords[0])
			{
				v.UV.x = mesh->mTextureCoords[0][i].x;
				v.UV.y = mesh->mTextureCoords[0][i].y;
			}

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
		eastl::vector<Ref<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, filepath);
		eastl::vector<Ref<Texture2D>> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, filepath);
		//eastl::vector<Ref<Texture2D>> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, filepath);

		Ref<Texture2D> albedo = diffuseMaps.size() > 0 ? diffuseMaps[0] : nullptr;
		Ref<Texture2D> normal = normalMaps.size() > 0 ? normalMaps[0] : nullptr;
		if (!normal)
			normal = heightMaps.size() > 0 ? heightMaps[0] : nullptr;

		uint32_t index = m_Submeshes.size();
		m_Submeshes.push_back({ nodeName, meshBuffer, albedo, normal });
	}
}
