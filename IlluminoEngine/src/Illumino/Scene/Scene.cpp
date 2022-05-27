#include "ipch.h"
#include "Scene.h"

#include "Illumino/Renderer/SceneRenderer.h"
#include "Component.h"

namespace IlluminoEngine
{
	Scene::Scene()
	{
	}

	Entity Scene::CreateEntity(const char* name)
	{
		entt::entity e = m_Registry.create();
		Entity entity(e, this);
		entity.AddComponent<TagComponent>().Tag = name;
		entity.AddComponent<TransformComponent>();
		m_EntityMap.emplace((uint32_t)e, entity);
		return entity;
	}

	void Scene::DeleteEntity(Entity entity)
	{
		m_Registry.destroy(entity);
		m_EntityMap.erase(entity);
	}

	void Scene::OnUpdateEditor(Timestep ts)
	{
		
	}

	void Scene::OnRenderEditor(const Camera& camera)
	{
		SceneRenderer::BeginScene(camera);
		{
			auto& view = m_Registry.view<TransformComponent, MeshComponent>();
			for (auto entity : view)
			{
				auto [trans, mesh] = view.get<TransformComponent, MeshComponent>(entity);
				if (mesh.MeshGeometry)
					SceneRenderer::SubmitMesh(mesh.MeshGeometry->GetSubmesh(mesh.SubmeshIndex), trans.GetTransform());
			}
		}
		SceneRenderer::EndScene();
	}
}
