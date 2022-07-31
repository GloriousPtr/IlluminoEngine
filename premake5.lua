workspace "Illumino"
    architecture "x64"
	startproject "IlluminoEd"
    
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
	{
		"MultiProcessorCompile"
	}

-- Include directories relavtive to root folder (solution directory)
IncludeDir = {}
IncludeDir["optick"] = "%{wks.location}/IlluminoEngine/vendor/optick/src"
IncludeDir["glm"] = "%{wks.location}/IlluminoEngine/vendor/glm"
IncludeDir["assimp"] = "%{wks.location}/IlluminoEngine/vendor/assimp/assimp/include"
IncludeDir["assimp_config"] = "%{wks.location}/IlluminoEngine/vendor/assimp/_config_headers"
IncludeDir["assimp_config_assimp"] = "%{wks.location}/IlluminoEngine/vendor/assimp/_config_headers/assimp"
IncludeDir["imgui"] = "%{wks.location}/IlluminoEngine/vendor/imgui"
IncludeDir["EASTL"] = "%{wks.location}/IlluminoEngine/vendor/EASTL/include"
IncludeDir["EABase"] = "%{wks.location}/IlluminoEngine/vendor/EABase/include/Common"
IncludeDir["stb_image"] = "%{wks.location}/IlluminoEngine/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/IlluminoEngine/vendor/entt/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/IlluminoEngine/vendor/ImGuizmo"
IncludeDir["half"] = "%{wks.location}/IlluminoEngine/vendor/half"

group "Dependencies"
	include "IlluminoEngine/vendor/optick"
	include "IlluminoEngine/vendor/assimp"
	include "IlluminoEngine/vendor/imgui"
	include "IlluminoEngine/vendor/EASTL"

group ""

include "IlluminoEngine"
include "IlluminoEd"
