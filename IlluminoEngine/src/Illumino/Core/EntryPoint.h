#include "ipch.h"

#include "Log.h"
#include "Application.h"

extern IlluminoEngine::Application* IlluminoEngine::CreateApplication();

int main()
{
	IlluminoEngine::Log::Init();

	IlluminoEngine::Application* application = IlluminoEngine::CreateApplication();

	application->Run();

	delete application;

	return 0;
}
