#pragma once

#include <IlluminoEngine.h>
#include <imgui/imgui.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/AssetPanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/ViewportPanel.h"

namespace IlluminoEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;

	private:
		SceneHierarchyPanel m_SceneHierarchyPanel;
		PropertiesPanel m_PropertiesPanel;
		AssetPanel m_AssetPanel;
		StatsPanel m_StatsPanel;
		ViewportPanel m_ViewportPanel;
		
		Ref<Scene> m_ActiveScene;
	};
}
