#pragma once

#include <IlluminoEngine.h>

#include <imgui/imgui.h>

#include "BasePanel.h"
#include "SceneHierarchyPanel.h"
#include "../Utils/EditorCamera.h"

namespace IlluminoEngine
{
	class ViewportPanel : public BasePanel
	{
	public:
		ViewportPanel();
		virtual ~ViewportPanel() = default;

		virtual void OnUpdate(Timestep ts);
		virtual void OnImGuiRender();

		void SetContext(Scene* scene, SceneHierarchyPanel* hierarchyPanel)
		{
			m_Scene = scene;
			m_SceneHierarchyPanel = hierarchyPanel;
		}

	private:
		ImVec2 m_ViewportSizeMin = { 0, 0 };
		ImVec2 m_ViewportSizeMax = { 0, 0 };
		ImVec2 m_ViewportBounds[2] = { {0, 0}, {0, 0} };
		bool m_ViewportHovered = false;
		int m_GizmoType = -1;

		Ref<EditorCamera> m_EditorCamera;
		Ref<RenderTexture> m_RenderTexture;
		Scene* m_Scene;
		SceneHierarchyPanel* m_SceneHierarchyPanel;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 10.0f;

		float m_MaxMoveSpeed = 10.0f;
	};
}
