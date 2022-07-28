#include "ipch.h"
#include "SceneRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Illumino/Scene/Scene.h"
#include "Illumino/Scene/Component.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Shader.h"
#include "Texture.h"

namespace IlluminoEngine
{
	static Ref<Shader> s_Shader;
	static glm::mat4 s_ViewProjection;
	static glm::vec4 s_CameraPosition;
	static eastl::vector<Entity> s_PointLights;
	static std::vector<MeshData> s_Meshes;

	void SceneRenderer::Init()
	{
		OPTICK_EVENT();
		
		s_Shader = Shader::Create("Assets/Shaders/TestShader.hlsl",
			{
				{"POSITION", ShaderDataType::Float3},
				{"NORMAL", ShaderDataType::Float3},
				{"TANGENT", ShaderDataType::Float3},
				{"BITANGENT", ShaderDataType::Float3},
				{"TEXCOORD", ShaderDataType::Float2}
			});
	}

	void SceneRenderer::Shutdown()
	{
		OPTICK_EVENT();

	}

	void SceneRenderer::BeginScene(const Camera& camera, const eastl::vector<Entity>& pointLights)
	{
		OPTICK_EVENT();

		// TODO: setup camera, lights, etc data
		s_ViewProjection = camera.GetProjection() * camera.GetView();
		s_CameraPosition = camera.GetTransform()[3];

		s_PointLights = pointLights;
	}

	void SceneRenderer::EndScene()
	{
		OPTICK_EVENT();

		RenderPass();
	}

	void SceneRenderer::SubmitMesh(Submesh& submesh, glm::mat4& transform)
	{
		OPTICK_EVENT();

		MeshData meshData = 
		{
			transform,
			submesh
		};

		s_Meshes.push_back(meshData);
	}

	void SceneRenderer::RenderPass()
	{
		OPTICK_EVENT();

		RenderCommand::ClearColor({ 0.042f, 0.042f, 0.042f, 1.0f });

		if (s_Meshes.empty())
			return;

		s_Shader->BindPipeline();

		{
			OPTICK_EVENT("CameraData Upload");

			struct CameraData
			{
				glm::mat4x4 u_ViewProjection = s_ViewProjection;
				glm::vec4 u_CameraPosition = s_CameraPosition;
			} cameraData;

			const size_t cameraDataAlignedSize = ALIGN(256, sizeof(CameraData));
			ILLUMINO_ASSERT(cameraDataAlignedSize <= 256, "Upload Camera Data size is greater than 256 bytes!");
			const uint64_t cameraDataGpuHandle = s_Shader->CreateBuffer("CameraData", cameraDataAlignedSize);
			s_Shader->UploadBuffer("CameraData", &cameraData, sizeof(CameraData), 0);

			s_Shader->BindConstant(2, cameraDataGpuHandle);
		}

		{
			OPTICK_EVENT("LightData Upload");

			struct PointLight
			{
				glm::vec4 LightPosition = glm::vec4(0.0f);
				glm::vec4 LightColor = glm::vec4(1.0f);
				glm::vec4 LightFactors = glm::vec4(0.0f);
			};

			struct LightData
			{
				PointLight pointLights[3];
				int pointLightsSize;
			};

			const size_t lightDataAlignedSize = ALIGN(256, sizeof(LightData));
			const uint64_t lightDataGpuHandle = s_Shader->CreateBuffer("LightData", lightDataAlignedSize);

			char* lightDataBuffer = new char[lightDataAlignedSize];
			LightData lightData = {};
			lightData.pointLightsSize = glm::min((int)s_PointLights.size(), 3);
			for (size_t i = 0; i < lightData.pointLightsSize; ++i)
			{
				auto& light = s_PointLights[i].GetComponent<PointLightComponent>();
				PointLight& pl = lightData.pointLights[i];
				pl.LightPosition = glm::vec4(s_PointLights[i].GetComponent<TransformComponent>().Translation, 1.0f);
				pl.LightColor = glm::vec4(light.Color, light.Intensity);
				pl.LightFactors.x = light.Radius;
			}

			memcpy(lightDataBuffer, &lightData, sizeof(lightData));
			s_Shader->UploadBuffer("LightData", lightDataBuffer, lightDataAlignedSize, 0);
			delete[] lightDataBuffer;

			s_Shader->BindConstant(3, lightDataGpuHandle);
		}


		const uint32_t meshCount = s_Meshes.size();

		const size_t meshAlignedSize = ALIGN(256, sizeof(glm::mat4));
		const uint64_t meshGpuHandle = s_Shader->CreateBuffer("Properties", meshAlignedSize * meshCount);
		const size_t meshBufferSize = meshAlignedSize * meshCount;
		char* meshBuffer = new char[meshBufferSize];
		for (size_t i = 0; i < meshCount; ++i)
		{
			auto& meshData = s_Meshes[i];
			glm::mat4 model = meshData.Transform;
			memcpy(meshBuffer + meshAlignedSize * i, &model, sizeof(glm::mat4));
		}
		s_Shader->UploadBuffer("Properties", meshBuffer, meshBufferSize, 0);
		delete[] meshBuffer;


		
		struct Material
		{
			glm::vec4 u_MRAO = glm::vec4(0.0, 1.0, 0.0, 0.0);
		};

		const size_t materialAlignedSize = ALIGN(256, sizeof(Material));
		const uint64_t materialGpuHandle = s_Shader->CreateBuffer("Material", materialAlignedSize * meshCount);
		const size_t materialBufferSize = materialAlignedSize * meshCount;
		char* materialBuffer = new char[materialBufferSize];
		for (size_t i = 0; i < meshCount; ++i)
		{
			auto& meshData = s_Meshes[i];
			Material material = {};
			material.u_MRAO.r = meshData.SubmeshData.Metalness;
			material.u_MRAO.g = meshData.SubmeshData.Roughness;
			memcpy(materialBuffer + materialAlignedSize * i, &material, sizeof(Material));
		}

		s_Shader->UploadBuffer("Material", materialBuffer, materialBufferSize, 0);
		delete[] materialBuffer;


		uint32_t index = 0;
		for (auto& mesh : s_Meshes)
		{
			if (mesh.SubmeshData.Albedo)
				mesh.SubmeshData.Albedo->Bind(0);
			
			if (mesh.SubmeshData.Normal)
				mesh.SubmeshData.Normal->Bind(1);
			
			s_Shader->BindConstant(4, meshGpuHandle + meshAlignedSize * index);
			s_Shader->BindConstant(5, materialGpuHandle + materialAlignedSize * index);

			RenderCommand::DrawIndexed(mesh.SubmeshData.Geometry);
			++index;
		}

		s_Meshes.clear();
	}
}
