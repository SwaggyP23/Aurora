#pragma once

// For use by Sandbox apps------------------------
#include "Core/Base.h"

#include "Core/Application.h"
#include "Core/AABB.h"
#include "Core/Log.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"
#include "Core/TimeStep.h"
#include "Core/Ray.h"
#include "Core/Random.h"

#include "Core/Layer.h"

#include "ImGui/ImGuiLayer.h"
#include "ImGui/ImGuizmo.h"

#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"
#include "Scene/SceneSerializer.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Scene/Components.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/RendererCaps.h"

#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Framebuffers.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexArray.h"

#include "Debugging/Instrumentation.h"
#include "Debugging/Timer.h"

#include "Editor/EditorCamera.h"

#include "Utils/Utils.h"