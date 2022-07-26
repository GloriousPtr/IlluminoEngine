#pragma once

#include <entt.hpp>

#include "Illumino/Core/Core.h"

namespace IlluminoEngine
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		virtual ~Entity() = default;

		template<typename T>
		T& AddComponent()
		{
			return m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponent()
		{
			ILLUMINO_ASSERT(HasComponent<T>(), "Entity doesn't have the component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			ILLUMINO_ASSERT(HasComponent<T>(), "Entity doesn't have the component");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		operator bool() const { return m_Scene && m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t) m_EntityHandle; }

		bool operator ==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator !=(const Entity& other) const { return !(*this == other); }

		Entity GetParent();
		void SetParent(Entity parent);
		void RemoveParent();

	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
	};
}
