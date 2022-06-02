#include <IlluminoEngine.h>
#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/glm/gtx/norm.hpp>

#include "Utils/EditorTheme.h"
#include "Illumino/Core/Application.h"
#include "Illumino/Renderer/GraphicsContext.h"
#include "Window.h"

namespace IlluminoEngine
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		EditorTheme::ApplyTheme();
		EditorTheme::SetStyle();
		EditorTheme::SetFont();
	}

	void EditorLayer::OnAttach()
	{
		m_ActiveScene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetSelectionContext(m_ActiveScene.get());

		RenderTextureSpec spec;
		spec.Width = 1920;
		spec.Height = 1080;
		m_RenderTexture = RenderTexture::Create(spec);

		m_EditorCamera = CreateRef<EditorCamera>(glm::radians(45.0f), (float)spec.Width / (float)spec.Height, 0.001f, 1000.0f);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_ActiveScene->OnUpdateEditor(ts);

		uint32_t width = m_ViewportSizeMax.x - m_ViewportSizeMin.x;
		uint32_t height = m_ViewportSizeMax.y - m_ViewportSizeMin.y;
		const RenderTextureSpec& spec = m_RenderTexture->GetSpecification();
		if (width != 0 && height != 0 && (width != spec.Width || height != spec.Height))
		{
			m_RenderTexture->Resize(width, height);
			m_EditorCamera->SetViewportSize(width, height);
		}

		m_MousePosition = *((glm::vec2*) &(ImGui::GetMousePos()));
		const glm::vec3& position = m_EditorCamera->GetPosition();
		float yaw = m_EditorCamera->GetYaw();
		float pitch = m_EditorCamera->GetPitch();

		bool moved = false;
		if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			const glm::vec2 change = (m_MousePosition - m_LastMousePosition) * m_MouseSensitivity * ts.GetSeconds();
			yaw += change.x;
			pitch = glm::clamp(pitch - change.y, -89.9f, 89.9f);

			glm::vec3 moveDirection = glm::vec3(0.0f);
			if (ImGui::IsKeyDown(ImGuiKey_W))
			{
				moved = true;
				moveDirection += m_EditorCamera->GetForward();
			}
			else if (ImGui::IsKeyDown(ImGuiKey_S))
			{
				moved = true;
				moveDirection -= m_EditorCamera->GetForward();
			}
			if (ImGui::IsKeyDown(ImGuiKey_D))
			{
				moved = true;
				moveDirection += m_EditorCamera->GetRight();
			}
			else if (ImGui::IsKeyDown(ImGuiKey_A))
			{
				moved = true;
				moveDirection -= m_EditorCamera->GetRight();
			}

			float maxMoveSpeed = m_MaxMoveSpeed * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 3.0f : 1.0f);
			m_EditorCamera->SetPosition(position + (ts * maxMoveSpeed * moveDirection));
			m_EditorCamera->SetYaw(yaw);
			m_EditorCamera->SetPitch(pitch);
		}

		m_EditorCamera->OnUpdate(ts);
		m_LastMousePosition = m_MousePosition;

		m_SceneHierarchyPanel.OnUpdate(ts);
		m_PropertiesPanel.OnUpdate(ts);
		m_StatsPanel.OnUpdate(ts);
	}

	static void BeginDockspace(const char* name)
	{
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(name, nullptr, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
	}

	static void EndDockspace()
	{
		ImGui::End();
	}

	void EditorLayer::OnImGuiRender()
	{
		m_RenderTexture->Bind();
		m_ActiveScene->OnRenderEditor(*m_EditorCamera);
		m_RenderTexture->Unbind();

		BeginDockspace("Dockspace");
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					ImGui::MenuItem("New");
					ImGui::MenuItem("Save");
					ImGui::MenuItem("Open");

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Viewport");
					ImGui::MenuItem("Heirarchy");
					ImGui::MenuItem("Properties");

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////
			//// Viewport ////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
			ImGui::Begin("Viewport");
			m_ViewportSizeMin = ImGui::GetWindowContentRegionMin();
			m_ViewportSizeMax = ImGui::GetWindowContentRegionMax();
			uint32_t width = m_ViewportSizeMax.x - m_ViewportSizeMin.x;
			uint32_t height = m_ViewportSizeMax.y - m_ViewportSizeMin.y;

			m_ViewportHovered = ImGui::IsWindowHovered();

			uint64_t textureID = m_RenderTexture->GetRendererID();
			ImGui::Image((ImTextureID)textureID, { (float)width, (float)height });

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const char* path = (const char*)payload->Data;
					eastl::string ext = StringUtils::GetExtension(path);

					if (ext == "assbin" || ext == "obj" || ext == "fbx")
					{
						Ref<Mesh> mesh = CreateRef<Mesh>(path);

						Entity parent = m_ActiveScene->CreateEntity(mesh->GetName());

						uint32_t meshCount = mesh->GetSubmeshCount();
						for (size_t i = 0; i < meshCount; i++)
						{
							auto& submesh = mesh->GetSubmesh(i);
							Entity entity = m_ActiveScene->CreateEntity(submesh.Name.c_str());
							entity.SetParent(parent);
							auto& meshComponent = entity.AddComponent<MeshComponent>();
							meshComponent.MeshGeometry = mesh;
							meshComponent.SubmeshIndex = i;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();

			bool b = true;
			ImGui::ShowDemoWindow(&b);

			m_SceneHierarchyPanel.OnImGuiRender();
			m_PropertiesPanel.SetSelectedEntity(m_SceneHierarchyPanel.GetSelection());
			m_PropertiesPanel.OnImGuiRender();
			m_AssetPanel.OnImGuiRender();
			m_StatsPanel.OnImGuiRender();
		}
		EndDockspace();
	}
}
