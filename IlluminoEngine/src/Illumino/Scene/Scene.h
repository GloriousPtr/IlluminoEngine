#pragma once

#include <entt.hpp>
#include <EASTL/hash_map.h>

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

		void OnUpdateEditor(Timestep ts);
		void OnRenderEditor(const Camera& camera);

		const eastl::hash_map<uint32_t, Entity>& GetEntityMap() const { return m_EntityMap; }

	private:
		friend class Entity;
		entt::registry m_Registry;
		eastl::hash_map<uint32_t, Entity> m_EntityMap;
	};
}
