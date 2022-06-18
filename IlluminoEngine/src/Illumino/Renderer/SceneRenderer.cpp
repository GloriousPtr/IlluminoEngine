#include "ipch.h"
#include "SceneRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "RenderCommand.h"
#include "Shader.h"
#include "Shader.h"
#include "Texture.h"

namespace IlluminoEngine
{
	static Ref<Shader> s_Shader;
	static glm::mat4 s_ViewProjection;
	
	std::vector<MeshData> SceneRenderer::s_Meshes;

	void SceneRenderer::Init()
	{
		OPTICK_EVENT();
		
		s_Shader = Shader::Create("Assets/Shaders/TestShader.hlsl",
			{
				{"POSITION", ShaderDataType::Float3},
				{"TEXCOORD", ShaderDataType::Float2}
			});
	}

	void SceneRenderer::Shutdown()
	{
		OPTICK_EVENT();

	}

	void SceneRenderer::BeginScene(const Camera& camera)
	{
		OPTICK_EVENT();

		// TODO: setup camera, lights, etc data
		s_ViewProjection = camera.GetProjection() * camera.GetView();
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
		
		struct CB
		{
			glm::mat4 u_MVP;
			glm::vec4 u_Color = { 1.0f, 0.0f, 0.0f, 1.0f };
		};

		const size_t alignedSize = ALIGN(256, sizeof(CB));
		const uint32_t meshCount = s_Meshes.size();

		uint64_t gpuHandle = s_Shader->CreateBuffer("Properties", alignedSize * meshCount);

		size_t bufferSize = alignedSize * meshCount;
		
		char* buffer = new char[bufferSize];
		for (size_t i = 0; i < meshCount; ++i)
		{
			auto& meshData = s_Meshes[i];
			CB cb;
			cb.u_MVP = s_ViewProjection * meshData.Transform;
			memcpy(buffer + alignedSize * i, &cb, sizeof(CB));
		}

		s_Shader->UploadBuffer("Properties", buffer, bufferSize, 0);
		delete[] buffer;

		uint32_t index = 0;
		for (auto& mesh : s_Meshes)
		{
			if (mesh.SubmeshData.Albedo)
				mesh.SubmeshData.Albedo->Bind(0);
			
			if (mesh.SubmeshData.Normal)
				mesh.SubmeshData.Normal->Bind(1);
			
			s_Shader->BindConstant(3, gpuHandle + alignedSize * index);

			RenderCommand::DrawIndexed(mesh.SubmeshData.Geometry);
			++index;
		}

		s_Meshes.clear();
	}
}
