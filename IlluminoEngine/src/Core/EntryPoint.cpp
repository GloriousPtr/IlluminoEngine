#include "ipch.h"

#include "Application.h"

int main()
{
	IlluminoEngine::Application* application = new IlluminoEngine::Application();

	application->Run();

	delete application;

	return 0;
}
