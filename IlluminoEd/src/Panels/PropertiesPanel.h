#pragma once

#include <IlluminoEngine.h>

#include "BasePanel.h"

namespace IlluminoEngine
{
	class PropertiesPanel : public BasePanel
	{
	public:
		PropertiesPanel() = default;
		virtual ~PropertiesPanel() = default;

		void OnUpdate(Timestep ts) {}
		void OnImGuiRender();

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

	private:
		void DrawComponents(Entity entity);

		template<typename Component>
		void DisplayAddComponentEntry(const char* entryName);

	private:
		Entity m_SelectedEntity;
	};
}
