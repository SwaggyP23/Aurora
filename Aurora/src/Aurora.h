#pragma once

// For use by Sandbox apps------------------------

#include <Windows.h> // Just to remove the dumb APIENTRY macro redefenitions

#include "Core/Application.h"

#include "Core/Base.h"
#include "Debugging/Instrumentor.h"

#include "Layers/Layer.h"
#include "Logging/Log.h"

#include "Utils/Random.h"

#include "Input/Input.h"
#include "Input/KeyCodes.h"

#include "ImGui/ImGuiLayer.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderer3D.h"
#include "Renderer/RenderCommand.h"

#include "Graphics/Buffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"

#include "Graphics/OrthoGraphicCamera.h"
#include "Graphics/EditorCamera.h"