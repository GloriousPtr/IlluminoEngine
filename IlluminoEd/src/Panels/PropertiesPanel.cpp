#include "PropertiesPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace IlluminoEngine
{
	void PropertiesPanel::OnImGuiRender()
	{
		if (OnBegin(ICON_MDI_PEN, "Properties"))
		{
			if (m_SelectedEntity)
				DrawComponents(m_SelectedEntity);

			OnEnd();
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const char* name, Entity entity, UIFunction fn, bool removable = true)
	{
		if (entity.HasComponent<T>())
		{
			static const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_FramePadding;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
			
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineHeight * 0.25f);

			bool opened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name);
			bool removeComponent = false;
			if (removable)
			{
				ImGui::SameLine(ImGui::GetContentRegionMax().x - lineHeight);
				if (ImGui::Button(ICON_MDI_SETTINGS, { lineHeight, lineHeight }))
					ImGui::OpenPopup("ComponentSettings");

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove Component"))
						removeComponent = true;

					ImGui::EndPopup();
				}
			}
			ImGui::PopStyleVar();

			if (opened)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().IndentSpacing / 2);
				fn(entity.GetComponent<T>());
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		ImGui::Spacing();

		// ID Component
		{
			UUID id = entity.GetComponent<IDComponent>().ID;
			ImGui::Text("UUID: %llu", (uint64_t)id);
		}

		ImGui::Spacing();

		// TagComponent
		{
			auto& tagComponent = entity.GetComponent<TagComponent>();
			char buffer[256];
			strcpy(buffer, tagComponent.Tag.c_str());
			if (ImGui::InputText("##Tag", buffer, 256))
				tagComponent.Tag = buffer;
		}

		ImGui::Spacing();

		DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component)
		{
			UI::BeginProperties();
			UI::DrawVec3Control("Translation", component.Translation);
			UI::DrawVec3Control("Rotation", component.Rotation);
			UI::DrawVec3Control("Scale", component.Scale);
			UI::EndProperties();
		}, false);

		DrawComponent<MeshComponent>("Mesh", entity, [](MeshComponent& component)
		{
			UI::BeginProperties();
			if (component.MeshGeometry)
			{
				UI::Property("Submesh Index", component.SubmeshIndex, 0, component.MeshGeometry->GetSubmeshCount() - 1);

				Submesh& submesh = component.MeshGeometry->GetSubmesh(component.SubmeshIndex);

				UI::Property("Albedo Map", submesh.Albedo);
				UI::Property("Normal Map", submesh.Normal);

				UI::Property("Roughness", submesh.Roughness, 0.0f, 1.0f);
				UI::Property("Metalness", submesh.Metalness, 0.0f, 1.0f);
			}
			UI::EndProperties();
		}, true);

		DrawComponent<PointLightComponent>("Point Light", entity, [](PointLightComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Intensity", component.Intensity);
			UI::PropertyColor3("Color", component.Color);
			UI::Property("Radius", component.Radius);
			UI::EndProperties();
		}, true);

		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](DirectionalLightComponent& component)
		{
			UI::BeginProperties();
			UI::Property("Intensity", component.Intensity);
			UI::PropertyColor3("Color", component.Color);
			UI::EndProperties();
		}, true);

		// AddComponent
		ImVec2 buttonSize = { 150, 30 };
		ImGui::SetCursorPos({ (ImGui::GetContentRegionMax().x - buttonSize.x) * 0.5f, ImGui::GetCursorPosY() + buttonSize.y * 0.5f });
		if (ImGui::Button("Add Component", buttonSize))
			ImGui::OpenPopup("AddComponentPopup");

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			DisplayAddComponentEntry<PointLightComponent>("Point Light");
			DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");

			ImGui::EndPopup();
		}
	}

	template<typename Component>
	void PropertiesPanel::DisplayAddComponentEntry(const char* entryName)
	{
		if (!m_SelectedEntity.HasComponent<Component>())
		{
			if (ImGui::MenuItem(entryName))
			{
				m_SelectedEntity.AddComponent<Component>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
