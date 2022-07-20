#pragma once

// For use by Sandbox apps------------------------

#include <Windows.h> // Just to remove the dumb APIENTRY macro redefenitions

#include "Core/Application.h"
#include "Core/Base.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

#include "Debugging/Instrumentation.h"
#include "Debugging/Timer.h"

#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"
#include "Graphics/Framebuffers.h"

#include "Graphics/EditorCamera.h"

#include "ImGui/ImGuiLayer.h"

#include "Layers/Layer.h"
#include "Logging/Log.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"

#include "Utils/Random.h"