#pragma once

#include "String.h"
#include "Timestep.h"

namespace IlluminoEngine
{
	class Layer
	{
	public:
		Layer(String&& name = "Layer")
			: m_DebugName((String&&)name)
		{
		}

		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}

		inline const String& GetName() const { return m_DebugName; }

	protected:
		String m_DebugName;
	};
}
