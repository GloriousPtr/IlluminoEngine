#include "ipch.h"
#include "RenderCommand.h"

namespace IlluminoEngine
{
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}
