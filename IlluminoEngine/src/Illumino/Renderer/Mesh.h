#pragma once

#include <EASTL/vector.h>
#include <EASTL/string.h>

#include "Buffer.h"
#include "Texture.h"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace IlluminoEngine
{
	struct Submesh
	{
		eastl::string Name;
		Ref<MeshBuffer> Geometry;
		Ref<Texture2D> Albedo;
	};

	class Mesh
	{
	public:
		Mesh(const char* filepath);
		virtual ~Mesh() = default;

		void Load(const char* filepath);

		Submesh& GetSubmesh(uint32_t index);
		const uint32_t GetSubmeshCount() const { return m_Submeshes.size(); }
		const char* GetName() const { return m_Name.c_str(); }

	private:
		void ProcessNode(aiNode *node, const aiScene *scene, const char* filepath);
		void ProcessMesh(aiMesh *mesh, const aiScene *scene, const char* filepath, const char* nodeName);

	private:
		eastl::string m_Name;
		eastl::vector<Submesh> m_Submeshes;
	};
}
