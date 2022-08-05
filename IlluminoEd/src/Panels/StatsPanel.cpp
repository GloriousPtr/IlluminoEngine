#include "StatsPanel.h"

#include <imgui/imgui.h>

#include <Window.h>
#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace IlluminoEngine
{
	void StatsPanel::OnImGuiRender()
	{
		OPTICK_EVENT();
		
		float avg = 0.0f;

		const uint32_t size = m_FrameTimes.size();
		if (size >= 50)
			m_FrameTimes.erase(m_FrameTimes.begin());

		m_FrameTimes.push_back(ImGui::GetIO().Framerate);
		for (uint32_t i = 0; i < size; i++)
		{
			m_FpsValues[i] = m_FrameTimes[i];
			avg += m_FrameTimes[i];
		}
		
		avg /= size;

		if (OnBegin(ICON_MDI_INFORMATION, "Stats"))
		{
			auto& graphicsContext = Application::GetApplication()->GetWindow()->GetGraphicsContext();
			UI::BeginProperties();
			bool vSync = graphicsContext->IsVsync();
			if (UI::Property("VSync Enabled", vSync))
				graphicsContext->SetVsync(vSync);
			UI::EndProperties();

			ImGui::Text("FPS");
			ImGui::Separator();
			ImGui::PlotLines("#FPS", m_FpsValues, size);
			ImGui::Text("FPS: %f", avg);
			const float fps = (1.0f / avg) * 1000.0f;
			ImGui::Text("Frame time (ms): %f", fps);

			OnEnd();
		}
	}
}
