#pragma once

#include <IlluminoEngine.h>

namespace IlluminoEngine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		virtual ~SceneHierarchyPanel() = default;

		void OnUpdate(Timestep ts);
		void OnImGuiRender();
		void SetSelectionContext(Scene* scene) { m_SelectionContext = scene; }
		void SetSelection(Entity entity) { m_SelectedEntity = entity; }

	private:
		void DrawEntityNode(Entity entity);

	private:
		Scene* m_SelectionContext;
		Entity m_SelectedEntity;
	};
}
