#pragma once

#include <entt.hpp>
#include <EASTL/hash_map.h>

#include "Illumino/Core/UUID.h"
#include "Illumino/Core/Timestep.h"
#include "Illumino/Renderer/Camera.h"
#include "Entity.h"

namespace IlluminoEngine
{
	class Scene
	{
	public:
		Scene();
		virtual ~Scene() = default;

		Entity CreateEntity(const char* name = "Entity");
		void DeleteEntity(Entity entity);

		Entity GetParent(Entity entity);
		void SetParent(Entity entity, Entity parent);
		void RemoveParent(Entity entity);

		void OnUpdateEditor(Timestep ts);
		void OnRenderEditor(const Camera& camera);

		const eastl::hash_map<UUID, Entity>& GetEntityMap() const { return m_EntityMap; }

	private:
		friend class Entity;
		entt::registry m_Registry;
		eastl::hash_map<UUID, Entity> m_EntityMap;
	};
}
