#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

namespace IlluminoEngine
{
	void SceneHierarchyPanel::OnUpdate(Timestep ts)
	{
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");

		if(m_SelectionContext)
		{
			const auto& entityMap = m_SelectionContext->GetEntityMap();
			for (auto [id, entity] : entityMap)
				DrawEntityNode(entity);

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Create"))
					m_SelectionContext->CreateEntity();

				if (m_SelectedEntity)
				{
					if (ImGui::MenuItem("Delete"))
					{
						m_SelectionContext->DeleteEntity(m_SelectedEntity);
						m_SelectedEntity = {};
					}
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth;

		const char* name = entity.GetComponent<TagComponent>().Tag.c_str();
		if (ImGui::TreeNodeEx((void*)(uint32_t)entity, treeFlags, name))
			ImGui::TreePop();
		
		if (ImGui::IsItemClicked())
			m_SelectedEntity = entity;
	}
}
