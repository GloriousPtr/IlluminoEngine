#include "ipch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace IlluminoEngine
{
	Ref<spdlog::logger> Log::s_Logger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;

		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("IlluminoEngine.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_Logger = CreateRef<spdlog::logger>("ILLUMINO_ENGINE", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
	}
}
