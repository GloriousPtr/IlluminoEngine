#pragma once

#include <IlluminoEngine.h>
#include <imgui/imgui.h>

#include "Utils/EditorCamera.h"

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
		Ref<EditorCamera> m_EditorCamera;
		Ref<RenderTexture> m_RenderTexture;

		ImVec2 m_ViewportSizeMin = { 0, 0 };
		ImVec2 m_ViewportSizeMax = { 0, 0 };
		bool m_ViewportHovered = false;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
		float m_MouseSensitivity = 0.1f;

		float m_MaxMoveSpeed = 1.0f;
		float m_MoveDampeningFactor = 0.000001f;
		glm::vec3 m_MoveDirection = glm::vec3(0.0f);
		float m_MoveVelocity = 0.0f;
	};
}
