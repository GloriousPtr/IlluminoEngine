#pragma once

namespace IlluminoEngine
{
	class Window;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		Ref<Window> m_Window;
	};
}
