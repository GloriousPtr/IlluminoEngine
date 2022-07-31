project "IlluminoEd"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/IlluminoEngine/vendor/spdlog/include",
		"%{wks.location}/IlluminoEngine/src",
		"%{wks.location}/IlluminoEngine/vendor",
		"%{IncludeDir.optick}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.assimp_config}",
		"%{IncludeDir.assimp_config_assimp}",
		"%{IncludeDir.EASTL}",
		"%{IncludeDir.EABase}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.half}",
	}

	links
	{
		"IlluminoEngine"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ILLUMINO_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ILLUMINO_DEBUG"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ILLUMINO_DEBUG"
		runtime "Release"
        optimize "on"
