#pragma once

#include <imgui/imgui.h>

namespace IlluminoEngine
{
	class BasePanel
	{
	public:
		BasePanel() = default;
		virtual ~BasePanel() = default;

		bool OnBegin(const char* icon, const char* name, ImGuiWindowFlags flags = 0);
		void OnEnd();

		bool& Showing() { return m_Showing; }

	private:
		bool m_Showing = true;
	};
}
