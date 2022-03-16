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


include "IlluminoEngine"
