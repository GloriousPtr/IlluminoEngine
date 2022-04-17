workspace "Illumino"
    architecture "x64"
	startproject "IlluminoEngine"
    
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
IncludeDir["assimp"] = "%{wks.location}/IlluminoEngine/vendor/assimp/include"

group "Dependencies"
	include "IlluminoEngine/vendor/optick"
	include "IlluminoEngine/vendor/assimp"

group ""

include "IlluminoEngine"
