#pragma once

// For use by Illumino Applications

//-----Core----------------------------------------
#include "Illumino/Core/Core.h"

#include "Illumino/Core/Application.h"
#include "Illumino/Core/Layer.h"
#include "Illumino/Core/Log.h"
#include "Illumino/Core/Assert.h"
#include "Illumino/Core/Timestep.h"

//-----ImGui---------------------------------------
#include "Illumino/ImGui/ImGuiLayer.h"

//-----Renderer------------------------------------
#include "Illumino/Renderer/SceneRenderer.h"
#include "Illumino/Renderer/RenderCommand.h"

#include "Illumino/Renderer/Buffer.h"
#include "Illumino/Renderer/Mesh.h"
#include "Illumino/Renderer/Shader.h"
#include "Illumino/Renderer/Texture.h"
#include "Illumino/Renderer/RenderTexture.h"
#include "Illumino/Renderer/Camera.h"

#include "Illumino/Scene/Scene.h"
#include "Illumino/Scene/Entity.h"
#include "Illumino/Scene/Component.h"

//-----Maths---------------------------------------
#include "Illumino/Math/Math.h"

//-----Profiling-----------------------------------
#include <optick.h>
