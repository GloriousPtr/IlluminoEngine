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
		Window* m_Window;
	};
}
