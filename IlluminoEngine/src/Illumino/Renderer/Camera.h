#pragma once

#include <glm/glm.hpp>

namespace IlluminoEngine
{
	class Camera
	{
	public:
		virtual ~Camera() = default;

		virtual const glm::mat4& GetView() const = 0;
		virtual const glm::mat4& GetProjection() const = 0;
	};
}
