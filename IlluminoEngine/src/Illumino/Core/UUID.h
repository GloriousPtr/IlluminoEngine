#pragma once

#include <EASTL/hash_map.h>

#include "Core.h"

namespace IlluminoEngine
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
}

namespace eastl
{
	template<>
	struct hash<IlluminoEngine::UUID>
	{
		std::size_t operator()(const IlluminoEngine::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
