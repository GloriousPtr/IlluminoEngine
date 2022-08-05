#pragma once

#include <IlluminoEngine.h>

#include "BasePanel.h"

namespace IlluminoEngine
{
	class StatsPanel : public BasePanel
	{
	public:
		StatsPanel() = default;
		virtual ~StatsPanel() = default;

		void OnUpdate(Timestep ts) {}
		void OnImGuiRender();

	private:
		float m_Time = 0.0f;
		float m_FpsValues[50];
		eastl::vector<float> m_FrameTimes;
	};
}
