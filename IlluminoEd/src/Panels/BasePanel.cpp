#include "BasePanel.h"

#include <IlluminoEngine.h>

#include <imgui/imgui_internal.h>

#include "../Utils/EditorTheme.h"

namespace IlluminoEngine
{
	bool BasePanel::OnBegin(const char* icon, const char* name, ImGuiWindowFlags flags)
	{
		if (!m_Showing)
			return false;

		ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);

		static eastl::string space = " ";
		static eastl::string tabs = "				";

		eastl::string title = space + icon + space + name + tabs;
		ImGui::Begin(title.c_str(), &m_Showing, flags | ImGuiWindowFlags_NoCollapse);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, EditorTheme::UIFramePadding);
		return true;
	}

	void BasePanel::OnEnd()
	{
		ImGui::PopStyleVar();
		ImGui::End();
	}
}
