#include "ipch.h"

#include "Log.h"
#include "Application.h"

int main()
{
	IlluminoEngine::Log::Init();

	IlluminoEngine::Application* application = new IlluminoEngine::Application();

	application->Run();

	delete application;

	return 0;
}
