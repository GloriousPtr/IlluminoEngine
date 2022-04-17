#include "Illumino/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace IlluminoEngine
{
	class IlluminoEd : public Application
	{
	public:
		IlluminoEd()
			: Application()
		{
			PushLayer(new EditorLayer());
		}

		~IlluminoEd()
		{
		}
	};

	Application* CreateApplication()
	{
		return new IlluminoEd();
	}
}
