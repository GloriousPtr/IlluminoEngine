#pragma once

#include <IlluminoEngine.h>

namespace IlluminoEngine
{
	class PropertiesPanel
	{
	public:
		PropertiesPanel() = default;
		virtual ~PropertiesPanel() = default;

		void OnUpdate(Timestep ts) {}
		void OnImGuiRender();

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

	private:
		void DrawComponents(Entity entity);

	private:
		Entity m_SelectedEntity;
	};
}
