#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include "Core.h"

namespace IlluminoEngine
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static Ref<spdlog::logger> s_Logger;
	};
}

#define ILLUMINO_TRACE(...)			IlluminoEngine::Log::GetLogger()->trace(__VA_ARGS__)
#define ILLUMINO_INFO(...)			IlluminoEngine::Log::GetLogger()->info(__VA_ARGS__)
#define ILLUMINO_WARN(...)			IlluminoEngine::Log::GetLogger()->warn(__VA_ARGS__)
#define ILLUMINO_ERROR(...)			IlluminoEngine::Log::GetLogger()->error(__VA_ARGS__)
#define ILLUMINO_CRITICAL(...)		IlluminoEngine::Log::GetLogger()->critical(__VA_ARGS__)
