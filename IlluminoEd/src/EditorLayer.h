#pragma once

#include <IlluminoEngine.h>
#include <imgui/imgui.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/AssetPanel.h"
#include "Panels/StatsPanel.h"
#include "Utils/EditorCamera.h"

namespace IlluminoEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;

	private:
		SceneHierarchyPanel m_SceneHierarchyPanel;
		PropertiesPanel m_PropertiesPanel;
		AssetPanel m_AssetPanel;
		StatsPanel m_StatsPanel;

		Ref<EditorCamera> m_EditorCamera;
		Ref<RenderTexture> m_RenderTexture;
		Ref<Scene> m_ActiveScene;

		ImVec2 m_ViewportSizeMin = { 0, 0 };
		ImVec2 m_ViewportSizeMax = { 0, 0 };
		bool m_ViewportHovered = false;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 0.1f;

		float m_MaxMoveSpeed = 10.0f;
	};
}
