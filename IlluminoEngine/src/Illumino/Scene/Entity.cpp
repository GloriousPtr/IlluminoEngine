#include "ipch.h"
#include "Entity.h"

#include "Scene.h"

namespace IlluminoEngine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	Entity Entity::GetParent()
	{
		return m_Scene->GetParent(*this);
	}
	
	void Entity::SetParent(Entity parent)
	{
		m_Scene->SetParent(*this, parent);
	}
	
	void Entity::RemoveParent()
	{
		if (GetParent())
			m_Scene->RemoveParent(*this);
	}
}
