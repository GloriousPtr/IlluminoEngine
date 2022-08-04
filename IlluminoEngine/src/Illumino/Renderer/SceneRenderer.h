#pragma once

#include <glm/glm.hpp>

#include "Illumino/Scene/Entity.h"
#include "Camera.h"
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
		static void BeginScene(const Camera& camera, const eastl::vector<Entity>& pointLights, const eastl::vector<Entity>& directionalLight);
		static void EndScene();

		static void SubmitMesh(Submesh& mesh, glm::mat4& transform);

	private:
		static void RenderPass();
	};
}
