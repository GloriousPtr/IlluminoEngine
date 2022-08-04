#include <IlluminoEngine.h>
#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/glm/gtx/norm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "Utils/UI.h"
#include "Utils/EditorTheme.h"
#include "Illumino/Core/Application.h"
#include "Illumino/Renderer/GraphicsContext.h"
#include "Window.h"

namespace IlluminoEngine
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		UI::Init();

		EditorTheme::ApplyTheme();
		EditorTheme::SetStyle();
		EditorTheme::SetFont();
	}
	
	EditorLayer::~EditorLayer()
	{
		UI::Shutdown();
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

		if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			const glm::vec2 change = (m_MousePosition - m_LastMousePosition) * m_MouseSensitivity;
			yaw += change.x;
			pitch = glm::clamp(pitch - change.y, -89.9f, 89.9f);

			float maxMoveSpeed = m_MaxMoveSpeed * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 3.0f : 1.0f);
			glm::vec3 pos = position;
			if (ImGui::IsKeyDown(ImGuiKey_W))
				pos += m_EditorCamera->GetForward() * m_MaxMoveSpeed * ts.GetSeconds();
			else if (ImGui::IsKeyDown(ImGuiKey_S))
				pos -= m_EditorCamera->GetForward() * m_MaxMoveSpeed * ts.GetSeconds();
			if (ImGui::IsKeyDown(ImGuiKey_D))
				pos += m_EditorCamera->GetRight() * m_MaxMoveSpeed * ts.GetSeconds();
			else if (ImGui::IsKeyDown(ImGuiKey_A))
				pos -= m_EditorCamera->GetRight() * m_MaxMoveSpeed * ts.GetSeconds();

			m_EditorCamera->SetPosition(pos);
			m_EditorCamera->SetYaw(yaw);
			m_EditorCamera->SetPitch(pitch);
		}

		m_LastMousePosition = m_MousePosition;
		m_EditorCamera->OnUpdate(ts);

		m_SceneHierarchyPanel.OnUpdate(ts);
		m_PropertiesPanel.OnUpdate(ts);
		m_StatsPanel.OnUpdate(ts);

		if (!ImGuizmo::IsUsing() && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Q))
				m_GizmoType = -1;
			if (ImGui::IsKeyPressed(ImGuiKey_W))
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_E))
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_R))
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
		}
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
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = { m_ViewportSizeMin.x + viewportOffset.x, m_ViewportSizeMin.y + viewportOffset.y };
			m_ViewportBounds[1] = {m_ViewportSizeMax.x + viewportOffset.x, m_ViewportSizeMax.y + viewportOffset.y};

			uint32_t width = m_ViewportSizeMax.x - m_ViewportSizeMin.x;
			uint32_t height = m_ViewportSizeMax.y - m_ViewportSizeMin.y;

			m_ViewportHovered = ImGui::IsWindowHovered();
			
			m_RenderTexture->Bind();
			m_ActiveScene->OnRenderEditor(*m_EditorCamera);
			m_RenderTexture->Unbind();
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

			// Gizmos
			if (m_ViewportHovered && m_GizmoType != -1)
			{
				Entity selectedEntity = m_SceneHierarchyPanel.GetSelection();
				if (selectedEntity)
				{
					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();

					ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

					const glm::mat4& cameraProjection = m_EditorCamera->GetProjection();
					const glm::mat4& cameraView = m_EditorCamera->GetView();
					// Entity Transform
					auto& tc = selectedEntity.GetComponent<TransformComponent>();
					auto& rc = selectedEntity.GetComponent<RelationshipComponent>();
					glm::mat4 transform = tc.GetTransform();

					// Snapping
					const bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
					float snapValue = 0.5f;
					if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
						snapValue = 45.0f;

					float snapValues[3] = { snapValue, snapValue, snapValue };

					ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

					if (m_ViewportHovered && ImGuizmo::IsUsing())
					{
						glm::mat4& parentWorldTransform = rc.Parent != 0 ? selectedEntity.GetParent().GetComponent<TransformComponent>().GetTransform() : glm::mat4(1.0f);
						glm::vec3 translation, rotation, scale;
						Math::DecomposeTransform(glm::inverse(parentWorldTransform) * transform, translation, rotation, scale);

						tc.Translation = translation;
						const glm::vec3 deltaRotation = rotation - tc.Rotation;
						tc.Rotation += deltaRotation;
						tc.Scale = scale;
					}
				}
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
