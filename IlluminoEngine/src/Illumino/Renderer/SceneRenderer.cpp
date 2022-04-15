#include "ipch.h"
#include "SceneRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "RenderCommand.h"
#include "Shader.h"

namespace IlluminoEngine
{
	static Ref<Shader> s_Shader;
	static glm::mat4 s_Projection;
	
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

	void SceneRenderer::BeginScene()
	{
		OPTICK_EVENT();

		// TODO: setup camera, lights, etc data
		s_Projection = glm::perspective(90.0f, 1920.0f / 1080.0f, 0.03f, 1000.0f);
	}

	void SceneRenderer::EndScene()
	{
		OPTICK_EVENT();

		RenderPass();
	}

	void SceneRenderer::SubmitMesh(const Ref<MeshBuffer>& mesh, glm::mat4& transform)
	{
		OPTICK_EVENT();

		MeshData meshData = 
		{
			transform,
			mesh
		};

		s_Meshes.push_back(meshData);
	}

	void SceneRenderer::RenderPass()
	{
		OPTICK_EVENT();

		for (auto& meshData : s_Meshes)
		{
			s_Shader->Bind();

			struct
			{
				glm::mat4 u_MVP;
				glm::vec4 u_Color = { 1.0f, 0.0f, 0.0f, 1.0f };
			} buffer;

			buffer.u_MVP = s_Projection * meshData.Transform;
			s_Shader->UploadBuffer("Properties", &buffer, sizeof(buffer));
			meshData.Mesh->Bind();

			RenderCommand::DrawIndexed(meshData.Mesh);
		}

		s_Meshes.clear();
	}
}
