<h1 align="left" style="border-bottom: none;">
  <a href="https://github.com/MohitSethi99/IlluminoEngine/">Illumino Game Engine</a>
</h1>

[![Build Status](https://github.com/MohitSethi99/IlluminoEngine/workflows/build/badge.svg)](https://github.com/MohitSethi99/IlluminoEngine/actions?workflow=build)
[![CodeQL](https://github.com/MohitSethi99/IlluminoEngine/workflows/CodeQL/badge.svg)](https://github.com/MohitSethi99/IlluminoEngine/actions?workflow=CodeQL)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_IlluminoEngine&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_IlluminoEngine)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_IlluminoEngine&metric=bugs)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_IlluminoEngine)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_IlluminoEngine&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_IlluminoEngine)

<p align="left">
  <img alt="platforms" src="https://img.shields.io/badge/platform-Windows-blue?style=flat-square"/>
  <img alt="GitHub" src="https://img.shields.io/github/license/MohitSethi99/IlluminoEngine?color=blue&style=flat-square">
  <img alt="size" src="https://img.shields.io/github/repo-size/MohitSethi99/IlluminoEngine?style=flat-square"/>
  <br/>
</p>

## About

Illumino Game Engine is an in-development game engine written in C++.

I develop it in my spare time as a personal project for learning purposes.

## Set up

- You can clone using git. Make sure you do a ```--recursive``` clone!
```
git clone --recursive https://github.com/MohitSethi99/IlluminoEngine.git
```
- It is built in a Windows environment, using Visual Studio 2022.
- Execute the script `GenerateProjectFiles.bat` to generate the solution and project files
- To enable DX12 debug messages on console window, you can uncomment the preprocessor defination ```ENABLE_DX12_DEBUG_MESSAGES``` in ```IlluminoEngine\premake5.lua```

Projects are generated with [Premake 5](https://github.com/premake/premake-core/releases).
