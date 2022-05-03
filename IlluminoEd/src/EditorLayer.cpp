#include <IlluminoEngine.h>
#include "EditorLayer.h"

#include <imgui/imgui.h>

namespace IlluminoEngine
{
	static std::vector<Ref<MeshBuffer>> s_Meshes;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		SetTheme();
	}

	void EditorLayer::OnAttach()
	{
		MeshLoader::LoadMesh("Assets/Meshes/primitives/cone.fbx", s_Meshes);
		MeshLoader::LoadMesh("Assets/Meshes/primitives/sphere.fbx", s_Meshes);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{

	}

	void EditorLayer::OnImGuiRender()
	{
		bool b = true;
		ImGui::ShowDemoWindow(&b);

		SceneRenderer::BeginScene();
		
		static uint32_t counter = 0;
		counter++;
		float temp = glm::abs(glm::sin(static_cast<float>(counter) / 64.0f));
		for (size_t i = 0; i < s_Meshes.size(); ++i)
		{
			int32_t multiplier = i % 2 == 0 ? 1 : -1;
			glm::mat4 t = glm::mat4(1.0f) * glm::translate(glm::vec3(4.0f * multiplier, 0.0f, -10.0f))
				* glm::rotate(counter * glm::radians(90.0f) / 60, glm::vec3(temp, 1.0 - temp, temp));
			SceneRenderer::SubmitMesh(s_Meshes[i], t);
		}
		
		SceneRenderer::EndScene();
	}

	void EditorLayer::SetTheme()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Assets/Fonts/opensans/OpenSans-Regular.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("Assets/Fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		io.Fonts->Build();
		
		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.24f, 0.25f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.21f, 0.23f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.84f, 0.34f, 0.17f, 0.14f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.28f, 0.14f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowMenuButtonPosition = 0;

		style->WindowPadding = ImVec2(4.0f, 3.0f);
		style->FramePadding = ImVec2(4.0f, 4.0f);
		style->ItemSpacing = ImVec2(6.0f, 3.0f);
		style->IndentSpacing = 12;
		style->ScrollbarSize = 12;
		style->GrabMinSize = 9;

		style->WindowBorderSize = 1.0f;
		style->ChildBorderSize = 1.0f;
		style->PopupBorderSize = 3.0f;
		style->FrameBorderSize = 1.0f;
		style->TabBorderSize = 1.0f;
		style->WindowRounding = 6.0f;
		style->ChildRounding = 0.0f;
		style->FrameRounding = 4.0f;
		style->PopupRounding = 4.0f;
		style->ScrollbarRounding = 12.0f;
		style->GrabRounding = 2.0f;
		style->TabRounding = 6.0f;
	}
}
