#pragma once

#include <IlluminoEngine.h>

namespace IlluminoEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;

	private:
		void SetTheme();

	private:
		Ref<RenderTexture> m_RenderTexture;
	};
}
