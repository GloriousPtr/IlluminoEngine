#pragma once

#include <EASTL/string.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace IlluminoEngine
{
	struct TagComponent
	{
		eastl::string Tag = "";
		
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
	};

	struct TransformComponent
	{
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		glm::mat4 GetTransform()
		{
			return glm::translate(glm::mat4(1.0f), Translation)
				 * glm::toMat4(glm::quat(Rotation))
				 * glm::scale(glm::mat4(1.0f), Scale);
		}
	};
}
