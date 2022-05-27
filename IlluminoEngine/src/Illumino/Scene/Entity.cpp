#include "ipch.h"
#include "Entity.h"

#include "Scene.h"

namespace IlluminoEngine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}
}
