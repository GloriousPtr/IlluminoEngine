#include <IlluminoEngine.h>
#include "EditorLayer.h"

#include <imgui/imgui.h>
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
		m_ViewportPanel.SetContext(m_ActiveScene.get(), &m_SceneHierarchyPanel);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_ActiveScene->OnUpdateEditor(ts);

		m_SceneHierarchyPanel.OnUpdate(ts);
		m_ViewportPanel.OnUpdate(ts);
		m_PropertiesPanel.OnUpdate(ts);
		m_StatsPanel.OnUpdate(ts);
	}

	static void BeginDockspace(const char* name)
	{
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoCloseButton;

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
					ImGui::MenuItem("Viewport", nullptr, &m_ViewportPanel.Showing());
					ImGui::MenuItem("Heirarchy", nullptr, &m_SceneHierarchyPanel.Showing());
					ImGui::MenuItem("Properties", nullptr, &m_PropertiesPanel.Showing());
					ImGui::MenuItem("Assets", nullptr, &m_AssetPanel.Showing());
					ImGui::MenuItem("Stats", nullptr, &m_StatsPanel.Showing());

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			bool b = true;
			ImGui::ShowDemoWindow(&b);

			m_SceneHierarchyPanel.OnImGuiRender();
			m_ViewportPanel.OnImGuiRender();
			m_PropertiesPanel.SetSelectedEntity(m_SceneHierarchyPanel.GetSelection());
			m_PropertiesPanel.OnImGuiRender();
			m_AssetPanel.OnImGuiRender();
			m_StatsPanel.OnImGuiRender();
		}
		EndDockspace();
	}
}
