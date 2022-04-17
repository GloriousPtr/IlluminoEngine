#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

namespace IlluminoEngine
{
	class ImGuiLayer
	{
	public:
		void OnAttach();
		void OnDetach();
		void Begin();
		void End();
	};
}
