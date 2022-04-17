#pragma once

#include "Buffer.h"

namespace IlluminoEngine
{
	class MeshLoader
	{
	public:
		static void LoadMesh(const char* filepath, std::vector<Ref<MeshBuffer>>& meshes);
	};
}
