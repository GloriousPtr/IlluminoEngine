#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"
#include "../Utils/IconsMaterialDesignIcons.h"

namespace IlluminoEngine
{
	void SceneHierarchyPanel::OnUpdate(Timestep ts)
	{
		if (m_DeleteEntity)
		{
			if (m_SelectedEntity == m_DeleteEntity)
				m_SelectedEntity = {};

			m_SelectionContext->DeleteEntity(m_DeleteEntity);
			m_DeleteEntity = {};
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		if (OnBegin(ICON_MDI_VIEW_LIST, "Hierarchy"))
		{
			m_HoveredEntity = {};

			float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
			float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
			float line_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
			UI::DrawRowsBackground(m_CurrentlyVisibleEntities, line_height, x1, x2, 0, 0, ImGui::GetColorU32(EditorTheme::WindowBgAlternativeColor));
			m_CurrentlyVisibleEntities = 0;

			if (m_SelectionContext)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
				const auto& entityMap = m_SelectionContext->GetEntityMap();
				for (auto [id, entity] : entityMap)
				{
					if (!entity.GetParent())
						DrawEntityNode(entity);
				}

				if (ImGui::IsWindowHovered())
				{
					if (ImGui::IsMouseDown(0) || (ImGui::IsMouseDown(1) && !m_HoveredEntity))
						m_SelectedEntity = {};
				}
				ImGui::PopStyleVar();

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 10, 10 });
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::BeginMenu("Create"))
					{
						if (ImGui::MenuItem("Entity"))
						{
							Entity e = m_SelectionContext->CreateEntity();
							if (m_SelectedEntity)
								e.SetParent(m_SelectedEntity);
							else
								m_SelectedEntity = e;
						}

						ImGui::EndMenu();
					}

					if (m_SelectedEntity)
					{
						if (ImGui::MenuItem("Delete"))
						{
							m_DeleteEntity = m_SelectedEntity;
						}
					}

					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();
			}
			OnEnd();
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		++m_CurrentlyVisibleEntities;

		auto& children = entity.GetComponent<RelationshipComponent>().Children;
		ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_SpanFullWidth;

		treeFlags |= m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0;
		treeFlags |= children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0;

		bool highlight = m_SelectedEntity == entity;
		if (highlight)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderHoveredColor);
		}

		static const eastl::string icon = ICON_MDI_CUBE_OUTLINE + eastl::string(" ");
		eastl::string name = icon + entity.GetComponent<TagComponent>().Tag.c_str();
		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, treeFlags, name.c_str());
		
		if (highlight)
			ImGui::PopStyleColor(2);
		else
			ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			m_HoveredEntity = entity;
			if (ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1))
				m_SelectedEntity = entity;
		}

		const auto& entityMap = m_SelectionContext->GetEntityMap();
		if (opened)
		{
			for (auto& child : children)
				DrawEntityNode(entityMap.at(child));

			ImGui::TreePop();
		}
	}
}
