#include "Illumino/Core/EntryPoint.h"

namespace IlluminoEngine
{
	class IlluminoEd : public Application
	{
	public:
		IlluminoEd()
			: Application()
		{
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
