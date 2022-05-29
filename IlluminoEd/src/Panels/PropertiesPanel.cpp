#include "PropertiesPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../Utils/UI.h"

namespace IlluminoEngine
{
	void PropertiesPanel::OnImGuiRender()
	{
		ImGui::Begin("Properties");

		if (m_SelectedEntity)
			DrawComponents(m_SelectedEntity);

		ImGui::End();
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
				if (ImGui::Button("o", { lineHeight, lineHeight }))
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
				fn(entity.GetComponent<T>());
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void PropertiesPanel::DrawComponents(Entity entity)
	{
		// TagComponent
		{
			auto& tagComponent = entity.GetComponent<TagComponent>();
			char buffer[256];
			strcpy(buffer, tagComponent.Tag.c_str());
			if (ImGui::InputText("##Tag", buffer, 256))
			{
				tagComponent.Tag = buffer;
			}
		}

		DrawComponent<TransformComponent>("Transform Component", entity, [](TransformComponent& component)
		{
			UI::BeginProperties();
			UI::DrawVec3Control("Translation", component.Translation);
			UI::DrawVec3Control("Rotation", component.Rotation);
			UI::DrawVec3Control("Scale", component.Scale);
			UI::EndProperties();
		}, false);

		DrawComponent<MeshComponent>("Mesh Component", entity, [](MeshComponent& component)
		{
			UI::BeginProperties();
			if (component.MeshGeometry)
				UI::Property("Submesh Index", component.SubmeshIndex, 0, component.MeshGeometry->GetSubmeshCount() - 1);
			UI::EndProperties();
		}, true);

		// AddComponent
		ImVec2 buttonSize = { 150, 30 };
		ImGui::SetCursorPos({ (ImGui::GetContentRegionMax().x - buttonSize.x) * 0.5f, ImGui::GetCursorPosY() + buttonSize.y * 0.5f });
		if (ImGui::Button("Add Component", buttonSize))
		{
			if (ImGui::BeginPopupContextWindow())
			{

				ImGui::EndPopup();
			}
		}
	}
}
