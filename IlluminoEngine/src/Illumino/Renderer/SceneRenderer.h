#pragma once

#include <glm/glm.hpp>

#include "Mesh.h"

namespace IlluminoEngine
{
	struct MeshData
	{
		glm::mat4 Transform;
		Submesh& SubmeshData;
	};

	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene();
		static void EndScene();

		static void SubmitMesh(Submesh& mesh, glm::mat4& transform);

	private:
		static void RenderPass();

	private:
		
		static std::vector<MeshData> s_Meshes;
	};
}
