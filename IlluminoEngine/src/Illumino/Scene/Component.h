#pragma once

#include <EASTL/string.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Illumino/Core/UUID.h"
#include "Illumino/Renderer/Mesh.h"
#include "Illumino/Renderer/Texture.h"

namespace IlluminoEngine
{
	struct IDComponent
	{
		UUID ID;
		
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct RelationshipComponent
	{
		UUID Parent = 0;
		eastl::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
	};

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

	struct MeshComponent
	{
		Ref<Mesh> MeshGeometry;
		uint32_t SubmeshIndex;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct PointLightComponent
	{
		float Intensity = 1.0f;
		glm::vec3 Color = glm::vec3(1.0f);
		float Radius = 1.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct DirectionalLightComponent
	{
		float Intensity = 1.0f;
		glm::vec3 Color = glm::vec3(1.0f);

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};
}
