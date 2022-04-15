#pragma once

#include <glm/glm.hpp>

#include "Buffer.h"

namespace IlluminoEngine
{
	struct MeshData
	{
		glm::mat4 Transform;
		Ref<MeshBuffer> Mesh;
	};

	class SceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene();
		static void EndScene();

		static void SubmitMesh(const Ref<MeshBuffer>& mesh, glm::mat4& transform);

	private:
		static void RenderPass();

	private:
		
		static std::vector<MeshData> s_Meshes;
	};
}
