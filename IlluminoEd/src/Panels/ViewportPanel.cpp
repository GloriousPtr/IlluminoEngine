#include "ViewportPanel.h"

#include <ImGuizmo.h>
#include <glm/glm/gtc/type_ptr.hpp>

#include "../Utils/EditorTheme.h"

namespace IlluminoEngine
{
	ViewportPanel::ViewportPanel()
	{
		RenderTextureSpec spec;
		spec.Width = 1920;
		spec.Height = 1080;
		m_RenderTexture = RenderTexture::Create(spec);

		m_EditorCamera = CreateRef<EditorCamera>(glm::radians(45.0f), (float)spec.Width / (float)spec.Height, 0.001f, 1000.0f);
	}

	void ViewportPanel::OnUpdate(Timestep ts)
	{
		uint32_t width = m_ViewportSizeMax.x - m_ViewportSizeMin.x;
		uint32_t height = m_ViewportSizeMax.y - m_ViewportSizeMin.y;
		const RenderTextureSpec& spec = m_RenderTexture->GetSpecification();
		if (width != 0 && height != 0 && (width != spec.Width || height != spec.Height))
		{
			m_RenderTexture->Resize(width, height);
			m_EditorCamera->SetViewportSize(width, height);
		}

		m_MousePosition = *((glm::vec2*)&(ImGui::GetMousePos()));
		if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			glm::vec3 pos = m_EditorCamera->GetPosition();
			float yaw = m_EditorCamera->GetYaw();
			float pitch = m_EditorCamera->GetPitch();

			const glm::vec2 change = (m_MousePosition - m_LastMousePosition) * m_MouseSensitivity * ts.GetSeconds();
			yaw += change.x;
			pitch = glm::clamp(pitch - change.y, -89.9f, 89.9f);

			float maxMoveSpeed = m_MaxMoveSpeed * (ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 3.0f : 1.0f);
			float deltaMultiplier = ts * m_MaxMoveSpeed;
			if (ImGui::IsKeyDown(ImGuiKey_W))
				pos += m_EditorCamera->GetForward() * deltaMultiplier;
			else if (ImGui::IsKeyDown(ImGuiKey_S))
				pos -= m_EditorCamera->GetForward() * deltaMultiplier;
			if (ImGui::IsKeyDown(ImGuiKey_D))
				pos += m_EditorCamera->GetRight() * deltaMultiplier;
			else if (ImGui::IsKeyDown(ImGuiKey_A))
				pos -= m_EditorCamera->GetRight() * deltaMultiplier;

			m_EditorCamera->SetPosition(pos);
			m_EditorCamera->SetYaw(yaw);
			m_EditorCamera->SetPitch(pitch);
		}
		m_EditorCamera->OnUpdate(ts);
		m_LastMousePosition = m_MousePosition;

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

	void ViewportPanel::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		if (OnBegin(ICON_MDI_TERRAIN, "Viewport"))
		{
			m_ViewportSizeMin = ImGui::GetWindowContentRegionMin();
			m_ViewportSizeMax = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = { m_ViewportSizeMin.x + viewportOffset.x, m_ViewportSizeMin.y + viewportOffset.y };
			m_ViewportBounds[1] = { m_ViewportSizeMax.x + viewportOffset.x, m_ViewportSizeMax.y + viewportOffset.y };

			uint32_t width = m_ViewportSizeMax.x - m_ViewportSizeMin.x;
			uint32_t height = m_ViewportSizeMax.y - m_ViewportSizeMin.y;

			m_ViewportHovered = ImGui::IsWindowHovered();

			m_RenderTexture->Bind();
			m_Scene->OnRenderEditor(*m_EditorCamera);
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

						Entity parent = m_Scene->CreateEntity(mesh->GetName());

						uint32_t meshCount = mesh->GetSubmeshCount();
						for (size_t i = 0; i < meshCount; i++)
						{
							auto& submesh = mesh->GetSubmesh(i);
							Entity entity = m_Scene->CreateEntity(submesh.Name.c_str());
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
			if (m_SceneHierarchyPanel && m_ViewportHovered && m_GizmoType != -1)
			{
				Entity selectedEntity = m_SceneHierarchyPanel->GetSelection();
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
			OnEnd();
		}
		ImGui::PopStyleVar();
	}
}
