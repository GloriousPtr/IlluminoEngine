#pragma once

#include "Illumino/Core/Log.h"

#include <filesystem>

#define EXPAND(x) x
#define STRINGIFY(x) #x

#ifdef ILLUMINO_DEBUG
	#define ILLUMINO_DEBUGBREAK() __debugbreak()
	#define ILLUMINO_ENABLE_ASSERTS
#else
	#define ILLUMINO_DEBUGBREAK()
#endif

// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
	#define FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define FUNC_SIG __func__
#else
	#define FUNC_SIG "FUNC_SIG unknown!"
#endif


#ifdef ILLUMINO_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define ILLUMINO_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { ILLUMINO##type##CRITICAL(msg, __VA_ARGS__); ILLUMINO_DEBUGBREAK(); } }
	#define ILLUMINO_INTERNAL_ASSERT_WITH_MSG(type, check, ...) ILLUMINO_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define ILLUMINO_INTERNAL_ASSERT_NO_MSG(type, check) ILLUMINO_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", STRINGIFY(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define ILLUMINO_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define ILLUMINO_INTERNAL_ASSERT_GET_MACRO(...) EXPAND( ILLUMINO_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ILLUMINO_INTERNAL_ASSERT_WITH_MSG, ILLUMINO_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define ILLUMINO_ASSERT(...) EXPAND( ILLUMINO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define ILLUMINO_CORE_ASSERT(...) EXPAND( ILLUMINO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define ILLUMINO_ASSERT(...)
	#define ILLUMINO_CORE_ASSERT(...)
#endif
