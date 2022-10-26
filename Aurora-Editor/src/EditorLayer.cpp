#include "EditorLayer.h"

#include "AssetManager/AssetManager.h"

#include "Editor/SceneHierarchyPanel.h"
#include "Editor/ShadersPanel.h"
#include "Editor/EditorConsolePanel.h"
#include "Editor/EditorSelectionManager.h"
#include "Panels/ContentBrowserPanel.h"

#include "ImGui/ImGuiUtils.h"
#include "ImGui/TreeNode.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

#include <math.h>

namespace Aurora {

#pragma region EditorLayerMainMethods

	// TODO: TEMPORARY
	static glm::vec3 albedoColor = { 1.0f, 1.0f, 1.0f };
	static float metalness;
	static float roughness;
	static float emission;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_EditorCamera(EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f))
	{
	}

	void EditorLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		EditorResources::Init();
		m_DisplayImage = Renderer::GetBlackTexture();

		m_PanelsLibrary = EditorPanelsLibrary::Create();

		// TODO: Maybe provide the active scene??
		Ref<SceneHierarchyPanel> sceneHierarchyPanel = m_PanelsLibrary->AddPanel<SceneHierarchyPanel>(PanelCategory::View, "SceneHierarchyPanel", "Scene Hierarchy", true, m_EditorScene, SelectionContext::Scene);
		sceneHierarchyPanel->SetEntityDeletedCallback([this](Entity entity) { OnEntityDeleted(entity); });

		m_EditorScene = Scene::Create("Editor Scene");
		m_ActiveScene = m_EditorScene;
		m_ViewportRenderer = SceneRenderer::Create(m_ActiveScene);
		m_Renderer2D = m_ViewportRenderer->GetRenderer2D();

		m_ViewportRenderer->SetLineWidth(m_LineWidth);

		m_PanelsLibrary->AddPanel<ShadersPanel>(PanelCategory::View, "ShadersPanel", "Shaders", false); // Not open by default
		m_PanelsLibrary->AddPanel<EditorConsolePanel>(PanelCategory::View, "ConsolePanel", "Console", true);
		m_PanelsLibrary->AddPanel<ContentBrowserPanel>(PanelCategory::View, "ContentBrowserPanel", "Content Browser", true);
		AR_CONSOLE_LOG_TRACE("Hey I am Up! apw9uecna 0re9uvn awoieuvyb a9ery8vbq 98vuyb  ae0ru9vn aoireuvnaiur piunrpviun pun ailrhv apiruvn");

		// TODO: Temporary untill Projects are a thing!
		AssetManager::Init();
		m_PanelsLibrary->Deserialize();
		// Default open scene for now...!
		OpenScene("SandboxProject/Assets/scenes/StaticMeshTest.aurora");

		// TODO: TEMPORARY
		class CameraScript : public ScriptableEntity
		{
			virtual void OnUpdate(TimeStep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				auto& rotation = GetComponent<TransformComponent>().Rotation;

				float speed = 10.0f;

				if (Input::GetControllers().empty())
				{
					const auto& [x, y] = Input::GetMousePosition();
					const glm::vec2& mouse{ x, y };
					const glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.002f;

					if (Input::IsKeyDown(AR_KEY_LEFT_SHIFT))
						speed *= 2.0f;
					else if (Input::IsKeyDown(AR_KEY_LEFT_CONTROL))
						speed *= 0.5f;

					if (Input::IsKeyDown(AR_KEY_W))
						translation.x += speed * ts;
					else if (Input::IsKeyDown(AR_KEY_S))
						translation.x -= speed * ts;
					if (Input::IsKeyDown(AR_KEY_A))
						translation.z -= speed * ts;
					else if (Input::IsKeyDown(AR_KEY_D))
						translation.z += speed * ts;
					if (Input::IsKeyDown(AR_KEY_Q))
						translation.y += speed * ts;
					else if (Input::IsKeyDown(AR_KEY_E))
						translation.y -= speed * ts;

					if (Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_RIGHT))
					{
						rotation.x += -delta.y;
						rotation.y += delta.x;
					}

					m_InitialMousePosition = mouse;
				}
				else
				{
					const Controller* controller = Input::GetController(0);
					if (Input::IsControllerPresent(controller->ID))
					{
						if (Input::IsControllerButtonPressed(controller->ID, (int)AR_GAMEPAD_BUTTON_A))
							speed *= 2.0f;
						else if (Input::IsControllerButtonPressed(controller->ID, (int)AR_GAMEPAD_BUTTON_B))
							speed *= 0.5f;

						float threshold = Input::GetControllerAxis(controller->ID, 3);
						if (threshold > 0.2f || threshold < -0.2f)
							translation.x += speed * ts * glm::abs(threshold) * (threshold > 0.0f ? -1 : 1);
						threshold = Input::GetControllerAxis(controller->ID, 2);
						if (threshold > 0.2f || threshold < -0.2f)
							translation.z += speed * ts * glm::abs(threshold) * (threshold > 0.0f ? 1 : -1);

						if (Input::IsControllerButtonPressed(controller->ID, (int)AR_GAMEPAD_LEFT_BUMPER))
							translation.y += speed * ts;
						else if (Input::IsControllerButtonPressed(controller->ID, (int)AR_GAMEPAD_RIGHT_BUMPER))
							translation.y -= speed * ts;

						float delta = Input::GetControllerAxis(controller->ID, 0);
						if (delta > 0.15f || delta < -0.15f)
							rotation.y += delta * ts;

						delta = Input::GetControllerAxis(controller->ID, 1);
						if (delta > 0.15f || delta < -0.15f)
							rotation.z += -delta * ts;
					}
				}
			}
		private:
			glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		};

		Entity cameraEntity = m_ActiveScene->GetEntityByName("Camera");
		cameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraScript>();
	}

	void EditorLayer::OnDetach()
	{
		AR_PROFILE_FUNCTION();

		EditorResources::Shutdown();
	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		AR_PROFILE_FUNCTION();
		AR_SCOPE_PERF("EditorLayer::OnUpdate");

		switch (m_SceneState)
		{
		    case SceneState::Edit:
		    {
				m_EditorCamera.SetActive(m_AllowViewportCameraEvents);
				m_EditorCamera.OnUpdate(ts);
				m_EditorScene->OnRenderEditor(m_ViewportRenderer, ts, m_EditorCamera, albedoColor, glm::vec3{ metalness, roughness, emission });

				OnRender2D();

		    	break;
		    }
		    case SceneState::Play:
		    {
				if (m_EditorCameraInRuntime)
				{
					m_EditorCamera.SetActive(m_ViewportHovered || m_AllowViewportCameraEvents);
					m_EditorCamera.OnUpdate(ts);
					m_RuntimeScene->OnRenderEditor(m_ViewportRenderer, ts, m_EditorCamera, glm::vec3(1.0f), glm::vec3(1.0f));

					OnRender2D();
				}
				else
				{
					m_RuntimeScene->OnRenderRuntime(m_ViewportRenderer, ts);
				}

		    	break;
		    }
		    case SceneState::Pause:
		    {
				m_EditorCamera.SetActive(m_ViewportHovered);
				m_EditorCamera.OnUpdate(ts);

				m_RuntimeScene->OnRenderRuntime(m_ViewportRenderer, ts);

		    	break;
		    }
		    case SceneState::Simulate:
		    {
				AR_CORE_ASSERT(false, "Not Implemented Until Physics!");

		    	break;
		    }
		}

		if (Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_LEFT) && !m_StartedRightClickInViewport && m_ViewportFocused && m_ViewportHovered)
			m_StartedRightClickInViewport = true;

		if (!Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_RIGHT))
			m_StartedRightClickInViewport = false;
	}

	void EditorLayer::OnTick()
	{
		const std::unordered_map<const char*, float>& mp = Application::GetApp().GetPerformanceProfiler()->GetPerFrameData();

		m_SortedTimerValues.clear();
		m_SortedTimerValues.reserve(mp.size());
		for (const auto& [name, val] : mp)
			m_SortedTimerValues.emplace_back(name, val);

		std::sort(m_SortedTimerValues.begin(), m_SortedTimerValues.end(), [](const std::tuple<const char*, float>& a, const std::tuple<const char*, float>& b) -> bool
		{
			return std::get<1>(a) > std::get<1>(b);
		});
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_PanelsLibrary->OnEvent(e);

		if (m_SceneState == SceneState::Edit)
		{
			if (m_AllowViewportCameraEvents)
				m_EditorCamera.OnEvent(e);
		}
		else if (m_SceneState == SceneState::Simulate)
		{
			if (m_AllowViewportCameraEvents)
				m_EditorCamera.OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
		dispatcher.Dispatch<WindowPathDropEvent>([this](WindowPathDropEvent& e) { return OnPathDrop(e); });
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (ImGuiUtils::IsWindowFocused("Viewport") || ImGuiUtils::IsWindowFocused("Scene Hierarchy"))
		{
			if (m_ViewportHovered && !Input::IsMouseButtonPressed(MouseButton::ButtonRight) && m_ActiveScene != m_RuntimeScene)
			{
				switch (e.GetKeyCode())
				{
				    case KeyCode::Q:
				    {
				    	m_GizmoType = -1;
				    	break;
				    }
				    case KeyCode::W:
				    {
				    	m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				    	break;
				    }
				    case KeyCode::E:
				    {
				    	m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				    	break;
				    }
				    case KeyCode::R:
				    {
				    	m_GizmoType = ImGuizmo::OPERATION::SCALE;
				    	break;
				    }
					case KeyCode::Space:
					{
						if (SelectionManager::GetSelectionCount(SelectionContext::Scene) == 0)
							return false;

						UUID selectedEntityID = SelectionManager::GetSelections(SelectionContext::Scene).front();
						Entity selectedEntity = m_ActiveScene->GetEntityWithUUID(selectedEntityID);
						m_EditorCamera.Focus(selectedEntity.Transform().Translation);
					}
				}
			}

			switch (e.GetKeyCode())
			{
				case KeyCode::Escape:
				{
					SelectionManager::DeselectAll();
					break;
				}
				case KeyCode::P:
				{
					if (Input::IsKeyDown(KeyCode::LeftAlt))
					{
						if (m_SceneState == SceneState::Edit)
							OnScenePlay();
						else
							OnSceneStop();
					}

					break;
				}
				case KeyCode::Delete:
				{
					// Intentional copy since deleting modifies the selectionMap
					std::vector<UUID> selectedEntities = SelectionManager::GetSelections(SelectionContext::Scene);
					for (UUID entityID : selectedEntities)
					{
						Entity entityToBeDeleted = m_ActiveScene->GetEntityWithUUID(entityID);
						// Could pass the uuid from the entity since the uuid should be the same in the selectionMap and the scene
						SelectionManager::Deselect(SelectionContext::Scene, entityToBeDeleted.GetUUID());
						m_EditorScene->DestroyEntity(entityToBeDeleted);
					}

					break;
				}
			}

			if (Input::IsKeyDown(KeyCode::LeftControl) && !Input::IsMouseButtonPressed(MouseButton::ButtonRight))
			{
				switch (e.GetKeyCode())
				{
				    case KeyCode::D:
				    {
						std::vector<UUID> selectedEntities = SelectionManager::GetSelections(SelectionContext::Scene);
						for (const UUID& entityID : selectedEntities)
						{
							Entity entity = m_ActiveScene->GetEntityWithUUID(entityID);
							Entity duplicate = m_ActiveScene->CopyEntity(entity);
							SelectionManager::Deselect(SelectionContext::Scene, entity.GetUUID());
							SelectionManager::Select(SelectionContext::Scene, duplicate.GetUUID());
						}
				    
				    	break;
				    }
				    case KeyCode::G:
				    {
				    	m_ViewportRenderer->GetOptions().ShowGrid = !m_ViewportRenderer->GetOptions().ShowGrid;
				    	break;
				    }
				    case KeyCode::N:
				    {
				    	NewScene();
				    	break;
				    }
				    case KeyCode::O:
				    {
				    	OpenScene();
				    	break;
				    }
				    case KeyCode::S:
				    {
				    	SaveScene();
				    	break;
				    }
				}
				
				if (Input::IsKeyDown(KeyCode::LeftShift))
				{
					switch (e.GetKeyCode())
					{
					    case KeyCode::S:
					    {
					    	SaveSceneAs();
					    	break;
					    }
					}
				}
			}

			if (m_SceneState == SceneState::Play && Input::IsKeyDown(KeyCode::Escape))
				Input::SetCursorMode(CursorMode::Normal);

			if (m_SceneState == SceneState::Play && Input::IsKeyDown(KeyCode::LeftAlt))
			{
				if (e.GetRepeatCount() == 0)
				{
					switch (e.GetKeyCode())
					{
					    case KeyCode::C:
					    {
					    	m_EditorCameraInRuntime = !m_EditorCameraInRuntime;
					    	break;
					    }
					}
				}
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (m_ActiveScene == m_RuntimeScene)
			return false;

		if (e.GetMouseButtonCode() != MouseButton::ButtonLeft)
			return false;

		if (!m_ViewportHovered)
			return false;

		if (Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsMouseButtonPressed(MouseButton::ButtonRight))
			return false;

		if (ImGuizmo::IsOver())
			return false;

		ImGui::ClearActiveID();

		std::vector<SelectionData> selectionData;

		auto [mouseX, mouseY] = GetMouseInViewportSpace();
		if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
		{
			const EditorCamera& camera = m_EditorCamera;
			auto [origin, direction] = CastRay(camera, mouseX, mouseY);

			auto staticMeshEntites = m_ActiveScene->GetAllEntitiesWith<StaticMeshComponent>();
			for (auto e : staticMeshEntites)
			{
				Entity entity{ e, m_ActiveScene.raw() };
				StaticMeshComponent& smc = entity.GetComponent<StaticMeshComponent>();

				Ref<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(smc.StaticMesh);
				if (!staticMesh)
					continue;

				std::vector<SubMesh>& subMeshes = staticMesh->GetMeshSource()->GetSubMeshes();
				constexpr float lastT = std::numeric_limits<float>::max();
				for (uint32_t i = 0; i < subMeshes.size(); i++)
				{
					SubMesh& subMesh = subMeshes[i];
					glm::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);

					Ray ray = {
						glm::inverse(transform * subMesh.Transform) * glm::vec4(origin, 1.0f),
						glm::inverse(glm::mat3(transform * subMesh.Transform)) * direction
					};

					float t;
					bool intersects = ray.IntersectsAABB(subMesh.BoundingBox, t);
					if (intersects)
					{
						const std::vector<Triangle>& triangleCache = staticMesh->GetMeshSource()->GetTriangleCache(i);
						for (const Triangle& triangle : triangleCache)
						{
							if (ray.IntersectsTriangle(triangle.V1.Position, triangle.V2.Position, triangle.V3.Position, t))
							{
								AR_CONSOLE_LOG_INFO("INTERSECTION: {0}, t = {1}", subMesh.NodeName, t);
								selectionData.push_back({ entity, &subMesh, t });
								break;
							}
						}
					}
				}
			}

			std::sort(selectionData.begin(), selectionData.end(), [](SelectionData& a, SelectionData& b) {return a.Distance < b.Distance; });

			bool ctrlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
			bool shiftDown = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);

			if (!ctrlDown)
				SelectionManager::DeselectAll();

			if (!selectionData.empty())
			{
				Entity entityy = selectionData.front().Entity;

				if (SelectionManager::IsSelected(SelectionContext::Scene, entityy.GetUUID()) && ctrlDown)
					SelectionManager::Deselect(SelectionContext::Scene, entityy.GetUUID());
				else
					SelectionManager::Select(SelectionContext::Scene, entityy.GetUUID());
			}
		}

		return false;
	}

	// TODO: Handle meshes/textures/skybox and whatever being dropped
	bool EditorLayer::OnPathDrop(WindowPathDropEvent& e)
	{
		// We can retrieve more than one path at a time however that needs special handling. For opening
		// scenes, only one scene at a time can be open at the moment, When support for multiple scenes to be open
		// at the same time, we can then go through the vector and open all the scenes if more than one was dropped
		// For Example...
		//for (const auto& path : e.GetDroppedPaths())
		//{
		//	AR_WARN_TAG("Editor", "{0}", path.string());
		//}

		if (e.GetDroppedPathCount() == 1)
		{
			const std::filesystem::path& path = e.GetDroppedPaths().front();
			if (path.extension().string() == ".aurora")
				OpenScene(path);
		}

		return true;
	}

	void EditorLayer::OnRender2D()
	{
		if (!m_ViewportRenderer->GetFinalPassImage())
			return;

		m_Renderer2D->BeginScene(m_EditorCamera.GetViewProjection(), m_EditorCamera.GetViewMatrix());
		m_Renderer2D->SetTargetRenderPass(m_ViewportRenderer->GetExternalCompositeRenderPass());

		// Render sprites...
		{
			// TODO: Handle in case sprite has a texture!
			auto spriteRendererView = m_ActiveScene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
			for (auto entity : spriteRendererView)
			{
				auto [transform, sprite] = spriteRendererView.get<TransformComponent, SpriteRendererComponent>(entity);

				m_Renderer2D->DrawRotatedQuad(transform.Translation, transform.Rotation, transform.Scale, sprite.Color);
			}

			auto circleRendererView = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
			for (auto entity : circleRendererView)
			{
				auto [transform, circle] = circleRendererView.get<TransformComponent, CircleRendererComponent>(entity);

				m_Renderer2D->FillCircle(transform.GetTransform(), circle.Color, circle.Thickness);
			}
		}

		// TODO: Maybe only the selected mesh???
		if (m_ShowBoundingBoxes)
		{
			auto staticMeshEntities = m_ActiveScene->GetAllEntitiesWith<StaticMeshComponent>();
			for (auto e : staticMeshEntities)
			{
				Entity entity{ e, m_ActiveScene.raw() };
				glm::mat4 transform = entity.GetComponent<TransformComponent>().GetTransform();
				Ref<StaticMesh> mesh = AssetManager::GetAsset<StaticMesh>(entity.GetComponent<StaticMeshComponent>().StaticMesh);
				if (mesh)
				{
					const AABB& aabb = mesh->GetMeshSource()->GetBoundingBox();
					m_Renderer2D->DrawAABB(aabb, transform);
				}
			}
		}

		if (m_ShowIcons)
		{
			auto cameraView = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
			for (auto entity : cameraView)
			{
				auto [transform, camera] = cameraView.get<TransformComponent, CameraComponent>(entity);

				m_Renderer2D->DrawRotatedQuad(transform.Translation, transform.Rotation, transform.Scale, EditorResources::CameraIcon, 1.0f, glm::vec4(1.0f));
			}
		}

		m_Renderer2D->EndScene();
	}

	std::pair<float, float> EditorLayer::GetMouseInViewportSpace()
	{
		auto [mx, my] = ImGui::GetMousePos();

		mx -= m_ViewportRect.Min.x;
		my -= m_ViewportRect.Min.y;
		ImVec2 viewportSize = { m_ViewportRect.Max.x - m_ViewportRect.Min.x, m_ViewportRect.Max.y - m_ViewportRect.Min.y };
		my = viewportSize.y - my; // Invert my

		return { (mx / viewportSize.x) * 2.0f - 1.0f, (my / viewportSize.y) * 2.0f - 1.0f };
	}

	std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(const EditorCamera& camera, float mx, float my)
	{
		glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		glm::mat4 inverseProj = glm::inverse(camera.GetProjection());
		glm::mat3 inverseView = glm::inverse(camera.GetViewMatrix());

		glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = camera.GetPosition();
		glm::vec3 rayDir = inverseView * glm::vec3(ray);

		return { rayPos, rayDir };
	}

#pragma endregion

#pragma region FileDialogs/Scene Helpers

	void EditorLayer::NewScene()
	{
		SelectionManager::DeselectAll();

		m_EditorScene->Clear();
		m_EditorScene = nullptr;
		m_ActiveScene = nullptr;

		m_EditorScene = Scene::Create("New Scene"); // Creating new scene
		m_PanelsLibrary->SetSceneContext(m_EditorScene);

		m_ActiveScene = m_EditorScene;

		m_SceneFilePath = std::string();
		m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f);

		if (m_ViewportRenderer)
			m_ViewportRenderer->SetScene(m_ActiveScene.raw());
	}

	void EditorLayer::OpenScene()
	{
		std::filesystem::path filepath = Utils::WindowsFileDialogs::OpenFileDialog("Aurora Scene (*.aurora)\0*.aurora\0");
		
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			AR_CORE_ERROR_TAG("EditorLayer", "Tried to open a scene that does not exist!");
			return;
		}

		if (path.extension().string() != ".aurora")
		{
			AR_WARN_TAG("EditorLayer", "Could not load '{0}' - not an Aurora scene file!", path.filename().string());
			
			return;
		}

		if (m_SceneState == SceneState::Play)
			OnSceneStop();
		//else if (m_SceneState == SceneState::Simulate)
			// TODO: HANDLE THIS CASE

		Ref<Scene> newScene = Scene::Create("New Scene");
		SceneSerializer serializer(newScene);

		if (serializer.DeSerializeFromText(path.string()))
		{
			m_EditorScene = newScene;

			m_ActiveScene = m_EditorScene;
			m_SceneFilePath = path.string();
			std::replace(m_SceneFilePath.begin(), m_SceneFilePath.end(), '\\', '/');

			m_PanelsLibrary->SetSceneContext(m_EditorScene);

			SelectionManager::DeselectAll();

			if (m_ViewportRenderer)
				m_ViewportRenderer->SetScene(m_ActiveScene.raw());
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_SceneFilePath.empty())
			SerializeScene(m_ActiveScene, m_SceneFilePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		std::filesystem::path filepath = Utils::WindowsFileDialogs::SaveFileDialog("Aurora Scene (*.aurora)\0*.aurora\0");

		if (filepath.empty())
			return;

		if (!filepath.has_extension())
			filepath += SceneSerializer::SceneExtension;

		if (!filepath.empty())
		{
			SerializeScene(m_ActiveScene, filepath);
			m_SceneFilePath = filepath.string();
			std::replace(m_SceneFilePath.begin(), m_SceneFilePath.end(), '\\', '/');
		}
	}

	void EditorLayer::SerializeScene(const Ref<Scene>& scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.SerializeToText(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		SelectionManager::DeselectAll();

		m_SceneState = SceneState::Play;

		m_RuntimeScene = Scene::Create();
		m_EditorScene->CopyTo(m_RuntimeScene);
		m_RuntimeScene->OnRuntimeStart();
		m_PanelsLibrary->SetSceneContext(m_RuntimeScene);
		m_ActiveScene = m_RuntimeScene;
	}

	void EditorLayer::OnSceneSimulate()
	{

	}

	void EditorLayer::OnSceneStop()
	{
		SelectionManager::DeselectAll();

		m_RuntimeScene->OnRuntimeStop();

		m_SceneState = SceneState::Edit;
		Input::SetCursorMode(CursorMode::Normal);

		// Reset runtime scene
		m_RuntimeScene = nullptr;
		m_PanelsLibrary->SetSceneContext(m_EditorScene);
		m_ActiveScene = m_EditorScene;
	}

#pragma endregion

#pragma region RendererPanels

	void EditorLayer::ShowRendererVendorInfoUI()
	{
		ImGui::Begin("Renderer Vendor", &m_ShowRendererVendorInfo, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Vendor: %s", Renderer::GetRendererCapabilities().Vendor.c_str());
		ImGui::Text("Renderer: %s", Renderer::GetRendererCapabilities().Renderer.c_str());
		ImGui::Text("OpenGL Version: %s", Renderer::GetRendererCapabilities().Version.c_str());
		ImGui::Text("GLSL Version: %s", Renderer::GetRendererCapabilities().GLSLVersion.c_str());
		ImGui::Text("Max Samples: %d", Renderer::GetRendererCapabilities().MaxSamples);
		ImGui::Text("Max Anisotropy: %.f", Renderer::GetRendererCapabilities().MaxAnisotropy);

		ImGui::End();
	}

	void EditorLayer::ShowRendererStatsUI()
	{
		ImGui::Begin("Renderer Stats", &m_ShowRenderStatsUI);

		m_Peak = std::max(m_Peak, ImGui::GetIO().Framerate);

		// TODO: SceneRenderer::GetStats() + All the stats from Renderer2D
		//ImGui::Text("Draw Calls: %d", Renderer3D::GetStats().DrawCalls);
		//ImGui::Text("Quad Count: %d", Renderer3D::GetStats().QuadCount);
		//ImGui::Text("Vertex Count: %d", Renderer3D::GetStats().GetTotalVertexCount());
		//ImGui::Text("Index Count: %d", Renderer3D::GetStats().GetTotalIndexCount());
		//ImGui::Text("Vertex Buffer Usage: %.3f Megabytes", Renderer3D::GetStats().GetTotalVertexBufferMemory() / (1024.0f * 1024.0f));

		static bool VSyncState = true;

		if (ImGui::Checkbox("V-Sync", &VSyncState))
		{
			Application::GetApp().GetWindow().SetVSync(VSyncState);
		}

		ImGui::Text("Controllers:");

		std::vector<int> contIds = Input::GetConnectedControllerIDs();

		std::vector<std::string> contNames;
		for (int id : contIds)
		{
			contNames.push_back(std::string(Input::GetControllerName(id)));
		}

		for (std::string name : contNames)
		{
			ImGui::Text("%s", name.c_str());
		}

		ImGui::End();
	}

#pragma endregion

#pragma region Materials

	void EditorLayer::ShowMaterialsPanel()
	{
		ImGui::Begin("Materials", &m_ShowMaterialsPanel);

		ImGuiUtils::BeginPropertyGrid();

		ImGuiUtils::ColorEdit3Control("Color", albedoColor);
		ImGuiUtils::PropertyFloat("Metalness", metalness, 0.01f, 0.0f, 1.0f);
		ImGuiUtils::PropertyFloat("Roughness", roughness, 0.01f, 0.0f, 1.0f);
		ImGuiUtils::PropertyFloat("Emission", emission, 0.01f, 0.0f, 1.0f);

		ImGuiUtils::EndPropertyGrid();

		ImVec2 ramainingSize = ImGui::GetContentRegionAvail();
		void* textureID = (void*)(uint64_t)m_ViewportRenderer->GetGeometryRenderPass()->GetSpecification().TargetFramebuffer->GetColorAttachment(1)->GetTextureID();
		ImGui::Image(textureID, { ramainingSize.x, ramainingSize.y / 2 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		textureID = (void*)(uint64_t)m_ViewportRenderer->GetGeometryRenderPass()->GetSpecification().TargetFramebuffer->GetColorAttachment(2)->GetTextureID();
		ImGui::Image(textureID, ImGui::GetContentRegionAvail(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
	}

#pragma endregion

#pragma region PerformancePanel

	void EditorLayer::ShowTimers()
	{
		for (const auto& [name, value] : m_SortedTimerValues)
		{
			ImGui::Text("%s, %.4f", name, value);
		}
	}

	void EditorLayer::ShowPerformanceUI()
	{
		ImGui::Begin("Performance", &m_ShowPerformance);

		ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame Time: %.3f ms", Application::GetApp().GetTimeStep().GetMilliSeconds());
		ImGui::Text("CPU Frame: %.3f ms", Application::GetApp().GetCPUTime());
		ImGui::Text("V Sync: %s", Application::GetApp().GetWindow().IsVSync() ? "On" : "Off");
		ImGui::Text("Peak FPS: %.f", m_Peak);

		constexpr ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx("CPU Timers (Milliseconds)", flags))
		{
			ShowTimers();

			ImGui::TreePop();
		}

		ImGui::End();
	}

#pragma endregion

#pragma region EditingPanel

	void EditorLayer::ShowEditPanelUI()
	{
		ImGui::Begin("Editor Style", &m_ShowEditingPanel);

		if (ImGuiUtils::PropertyGridHeader("Fonts"))
		{
			ShowFontPickerUI();

			ImGui::TreePop();
		}

		if (ImGuiUtils::PropertyGridHeader("Editor Style"))
		{
			ImGui::ShowStyleEditor();

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void EditorLayer::ShowFontPickerUI()
	{

		ImGuiFontsLibrary& fontsLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;

		ImGuiUtils::BeginPropertyGrid();

		static const char* fonts[] = { "RobotoLarge", "RobotoBold", "RobotoDefault", "MochiyPopOne" };
		static int selected = 2;
		if (ImGuiUtils::PropertyDropdown("Available Fonts", fonts, 4, &selected))
		{
			fontsLib.SetDefaultFont(fonts[selected]);
			AR_DEBUG("Selected int: {0}", selected);
		}

		ImGuiUtils::EndPropertyGrid();
	}

	void EditorLayer::ShowScreenshotPanel()
	{
		ImGui::Begin("Screenshot Panel", &m_ShowScreenshotPanel);

		// Delay the screenshot taking until the frame has rendered
		if (ImGui::Button("Take Screenshot"))
		{
			AR_CHECK(false, "Not Working! Maybe need to delay taking the screenshot until the frame is done?");
			TakeScreenShotOfOpenScene();
		}

		ImGui::SameLine();

		ImGuiUtils::ShowHelpMarker("Press the button to take a screenshot of the currently open scene.");

		ImGui::Text("Output path: Luna/SandboxProject/Screenshots");

		ImVec2 size = ImGui::GetContentRegionAvail();
		uint64_t textureID = (uint64_t)m_DisplayImage->GetTextureID();
		ImGui::Image((void*)textureID, size, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

		ImGui::End();
	}

	void EditorLayer::TakeScreenShotOfOpenScene()
	{
		// TODO: Change the hard coded numbers to the width and height of the framebuffers
		Buffer buff(m_ViewportWidth * m_ViewportHeight * 4);
		m_ViewportRenderer->GetFinalPassImageData(buff.Data); // Returns the id of the image

		std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
		std::time_t timePoint = std::chrono::system_clock::to_time_t(currentTime);

		char buffer[64]{};
		ctime_s(buffer, 64, &timePoint);

		std::string_view str(buffer);
		size_t colonPos = str.find_first_of(':');
		std::string timeString(str.substr(colonPos - 2, 13));

		std::string finalName = "SandboxProject/Screenshots/" + m_ActiveScene->GetName() + "_" + timeString + ".png";
		for (uint32_t i = 21; i < finalName.size(); i++)
		{
			if (finalName[i] == ' ' || finalName[i] == ':')
				finalName[i] = '_';
		}

		//m_DisplayImage = Texture2D::Create(ImageFormat::RGBA, spec.Width, spec.Height, buff.Data);
		// If stb writes the image correctly i know that everything is good and i create the texture
		if (Utils::ImageLoader::WriteDataToPNGImage(finalName, buff.Data, m_ViewportWidth, m_ViewportHeight, 4, true))
		{
			AR_WARN_TAG("EditorLayer", "Wrote image to {0}", finalName);
			m_DisplayImage = Texture2D::Create(finalName);
		}
	}

#pragma endregion

#pragma region HelpPanels/UI

	void EditorLayer::ShowEditorCameraHelpUI()
	{
		ImGui::Begin("Editor Camera Controls:", &m_ShowEditorCameraHelpUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("These are the editor camera's controls:");
		ImGui::Text("RightClick + Use W/A/S/D/Q/E to control the camera");
		ImGui::Text("RightClick + MouseScroll to increase the movement speed of fps Camera");
		ImGui::Text("Press LeftAlt + RightClick and move to Rotate.");
		ImGui::Text("Press LeftAlt + LeftClick and move to Pan.");
		ImGui::Text("Press LeftAlt + ScrollWheel to zoom.");
		ImGui::Text("Press LeftAlt + MouseMiddleButton and move to Zoom.");
		ImGui::Text("Press LeftAlt + F key to reset the focal point.");
		ImGui::Text("Press LeftAlt + C to reset all the camera.");

		ImGui::End();
	}

#pragma endregion

#pragma region EditorLayerPrimaryPanels

	void EditorLayer::EnableDocking()
	{
		static bool dockSpaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		constexpr static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWindowSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f; // Minimum Window Size!
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWindowSizeX;
	}

	void EditorLayer::ShowMenuBarItems()
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 100));

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				// TODO: Convert all panels to this! For now: SceneHierarchy and Shaders only...
				{
					for (auto& [id, panelSpec] : m_PanelsLibrary->GetPanels(PanelCategory::View))
					{
						if (ImGui::MenuItem(panelSpec.Name, nullptr, &panelSpec.IsOpen))
							m_PanelsLibrary->Serialize();
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Materials", nullptr, m_ShowMaterialsPanel))
					m_ShowMaterialsPanel = !m_ShowMaterialsPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Performance", nullptr, m_ShowPerformance))
					m_ShowPerformance = !m_ShowPerformance;

				ImGui::Separator();

				if (ImGui::MenuItem("Renderer Stats", nullptr, m_ShowRenderStatsUI))
					m_ShowRenderStatsUI = !m_ShowRenderStatsUI;

				if (ImGui::MenuItem("Renderer Info", nullptr, m_ShowRendererVendorInfo))
					m_ShowRendererVendorInfo = !m_ShowRendererVendorInfo;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Screenshot", nullptr, m_ShowScreenshotPanel))
					m_ShowScreenshotPanel = !m_ShowScreenshotPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Editor Style", nullptr, m_ShowEditingPanel))
					m_ShowEditingPanel = !m_ShowEditingPanel;

				if (ImGui::MenuItem("ImGui StackTool", nullptr, m_ShowDearImGuiStackToolWindow))
					m_ShowDearImGuiStackToolWindow = !m_ShowDearImGuiStackToolWindow;

#ifdef AURORA_DEBUG
				if (ImGui::MenuItem("ImGui DebugLog", nullptr, m_ShowDearImGuiDebugLogWindow))
					m_ShowDearImGuiDebugLogWindow = !m_ShowDearImGuiDebugLogWindow;
#endif

				if (ImGui::MenuItem("ImGui Metrics", nullptr, m_ShowDearImGuiMetricsWindow))
					m_ShowDearImGuiMetricsWindow = !m_ShowDearImGuiMetricsWindow;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Settings...", nullptr, m_ShowSettingsUI))
					m_ShowSettingsUI = !m_ShowSettingsUI;

				ImGui::Separator();

				if (ImGui::MenuItem("Exit...", nullptr, m_ShowCloseModal))
					m_ShowCloseModal = !m_ShowCloseModal;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Editor Camera", nullptr, m_ShowEditorCameraHelpUI))
					m_ShowEditorCameraHelpUI = !m_ShowEditorCameraHelpUI;

				ImGui::EndMenu();
			}

			ImGui::PopStyleColor();

			ImGui::EndMenuBar();
		}
	}

	float EditorLayer::GetSnapValue()
	{
		switch (m_GizmoType)
		{
		    case ImGuizmo::OPERATION::TRANSLATE: return 0.5f;
			case ImGuizmo::OPERATION::ROTATE:    return 45.0f;
			case ImGuizmo::OPERATION::SCALE:     return 0.5f;
		}

		AR_ASSERT(false, "Unknown Gizmo operation type");
		return 0.0f;
	}

	void EditorLayer::DrawGizmos()
	{
		AR_PROFILE_FUNCTION();

		if (m_GizmoType == -1)
			return;

		const std::vector<UUID>& selections = SelectionManager::GetSelections(SelectionContext::Scene);

		if (selections.size() == 0)
			return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		bool snapping = Input::IsKeyDown(KeyCode::LeftControl);

		const float snapValue = GetSnapValue();
		const float snapValues[] = { snapValue, snapValue, snapValue };

		if (selections.size() == 1)
		{
			// Get first selection since there is only 1
			Entity entity = m_ActiveScene->GetEntityWithUUID(selections[0]);
			TransformComponent& tc = entity.Transform();
			glm::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);

			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			if (m_SceneState == SceneState::Play && !m_EditorCameraInRuntime)
			{
				Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				SceneCamera& sceneCamera = cameraEntity.GetComponent<CameraComponent>().Camera;
				projectionMatrix = sceneCamera.GetProjection();
				viewMatrix = glm::inverse(m_ActiveScene->GetWorldSpaceTransformMatrix(cameraEntity));
			}
			else
			{
				projectionMatrix = m_EditorCamera.GetProjection();
				viewMatrix = m_EditorCamera.GetViewMatrix();
			}

			if (ImGuizmo::Manipulate(
				glm::value_ptr(viewMatrix),
				glm::value_ptr(projectionMatrix),
				(ImGuizmo::OPERATION)m_GizmoType,
				ImGuizmo::MODE::LOCAL,
				glm::value_ptr(transform),
				nullptr,
				snapping ? snapValues : nullptr))
			{
				// TODO: Get Parent Entity...

				glm::vec3 Translation;
				glm::quat Rotation;
				glm::vec3 Scale;
				Math::DecomposeTransform(transform, Translation, Rotation, Scale);

				switch (m_GizmoType)
				{
				    case ImGuizmo::OPERATION::TRANSLATE:
				    {
				    	tc.Translation = Translation;
				    	break;
				    }
				    case ImGuizmo::OPERATION::ROTATE:
				    {
						// TODO: Try to do this with more precision by maybe storing the quaternion on the TransformComponent?
				    	tc.Rotation += glm::eulerAngles(Rotation) - tc.Rotation;
				    	break;
				    }
				    case ImGuizmo::OPERATION::SCALE:
				    {
				    	tc.Scale = Scale;
				    	break;
				    }
				}
			}
		}
		else
		{
			// TODO: Handle moving parented and child entities
		}
	}

	void EditorLayer::DrawGizmosToolBar()
	{
		// TODO: Maybe reduce the width and height of the overlay since it is kind of way too big, maybe try and fiddle with that after adding the 
		// buttons since that might look better!
		// Viewport Gizmo Tools
		{
			ImGuiScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
			ImGuiScopedStyle windowBorderSize(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGuiScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 8.0f);
			ImGuiScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

			constexpr float buttonSize = 18.0f + 5.0f;
			constexpr float edgeOffset = 4.0f;
			constexpr float windowHeight = 32.0f; // ImGui does not allow for the window to be less than 32.0f pixels
			constexpr float numberOfButtons = 3.0f;
			constexpr float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) /** 2.0f*/;

			float toolbarX = m_ViewportRect.Min.x + edgeOffset + numberOfButtons * buttonSize;
			ImGui::SetNextWindowPos(ImVec2{ toolbarX - (backgroundWidth / 2.0f), m_ViewportRect.Min.y + edgeOffset });
			ImGui::SetNextWindowSize(ImVec2{ backgroundWidth, windowHeight });
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::Begin("##viewportGizmoToolsControls", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

			// A hack to make icon panel appear smaller than minimum allowed by ImGui size
			// Filling the background for the desired 26px height
			constexpr float desiredHeight = 26.0f + 5.0f;
			ImRect background = ImGuiUtils::RectExpanded(ImGui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

			ImGui::BeginVertical("##viewportGizmoToolsControlsV", ImVec2{ backgroundWidth, ImGui::GetContentRegionAvail().y });
			ImGui::Spring();
			ImGui::BeginHorizontal("##viewportGizmoToolsControlsH", ImVec2{ backgroundWidth, ImGui::GetContentRegionAvail().y });
			ImGui::Spring();

			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ edgeOffset * 2.0f, 0.0f });

				auto toolbarButton = [buttonSize](const Ref<Texture2D>& icon, const ImColor& tint, float paddingY = 0.0f) -> bool
				{
					const float height = std::min((float)icon->GetHeight(), buttonSize) - paddingY * 2.0f;
					const float width = (float)icon->GetWidth() / (float)icon->GetHeight() * height;
					const bool clicked = ImGui::InvisibleButton(ImGuiUtils::GenerateID(), ImVec2{ width, height });
					ImGuiUtils::DrawButtonImage(icon, tint, tint, tint, ImGuiUtils::RectOffset(ImGuiUtils::GetItemRect(), 0.0f, paddingY));

					return clicked;
				};

				ImColor tintColor = m_GizmoType == ImGuizmo::OPERATION::TRANSLATE ? Theme::AccentHighlighted : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
				if (toolbarButton(EditorResources::TranslateIcon, tintColor))
				{
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip("Translate");
					ImGui::PopStyleVar(2);
				}

				tintColor = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? Theme::AccentHighlighted : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
				if (toolbarButton(EditorResources::RotateIcon, tintColor))
				{
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip("Rotate");
					ImGui::PopStyleVar(2);
				}

				tintColor = m_GizmoType == ImGuizmo::OPERATION::SCALE ? Theme::AccentHighlighted : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
				if (toolbarButton(EditorResources::ScaleIcon, tintColor))
				{
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip("Scale");
					ImGui::PopStyleVar(2);
				}

				ImGui::PopStyleVar();
			}

			ImGui::Spring();
			ImGui::EndHorizontal();
			ImGui::Spring();
			ImGui::EndVertical();

			ImGui::End();
		}
	}

	void EditorLayer::DrawCentralBar()
	{
		// Viewport Scene Tools
		{
			ImGuiScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
			ImGuiScopedStyle windowBorderSize(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGuiScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 8.0f);
			ImGuiScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

			constexpr float buttonSize = 18.0f + 5.0f;
			constexpr float edgeOffset = 4.0f;
			constexpr float windowHeight = 32.0f; // ImGui does not allow for the window to be less than 32.0f pixels
			constexpr float numberOfButtons = 3.0f;
			constexpr float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

			float toolbarX = (m_ViewportRect.Min.x + m_ViewportRect.Max.x) / 2.0f;
			ImGui::SetNextWindowPos(ImVec2{ toolbarX - (backgroundWidth / 2.0f), m_ViewportRect.Min.y + edgeOffset });
			ImGui::SetNextWindowSize(ImVec2{ backgroundWidth, windowHeight });
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::Begin("##viewportSceneControls", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

			// A hack to make icon panel appear smaller than minimum allowed by ImGui size
			// Filling the background for the desired 26px height
			constexpr float desiredHeight = 26.0f + 5.0f;
			ImRect background = ImGuiUtils::RectExpanded(ImGui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

			ImGui::BeginVertical("##viewportSceneControlsV", ImVec2{ backgroundWidth, ImGui::GetContentRegionAvail().y });
			ImGui::Spring();
			ImGui::BeginHorizontal("##viewportSceneControlsH", ImVec2{ backgroundWidth, ImGui::GetContentRegionAvail().y });
			ImGui::Spring();

			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ edgeOffset * 2.0f, 0.0f });

				auto toolbarButton = [buttonSize](const Ref<Texture2D>& icon, const ImColor& tint, float paddingY = 0.0f) -> bool
				{
					const float height = std::min((float)icon->GetHeight(), buttonSize) - paddingY * 2.0f;
					const float width = (float)icon->GetWidth() / (float)icon->GetHeight() * height;
					const bool clicked = ImGui::InvisibleButton(ImGuiUtils::GenerateID(), ImVec2{ width, height });
					ImGuiUtils::DrawButtonImage(icon, tint, tint, tint, ImGuiUtils::RectOffset(ImGuiUtils::GetItemRect(), 0.0f, paddingY));

					return clicked;
				};

				Ref<Texture2D> buttonTex = m_SceneState == SceneState::Play ? EditorResources::StopButton : EditorResources::PlayButton;
				if (toolbarButton(buttonTex, Theme::Text))
				{
					if (m_SceneState == SceneState::Edit)
						OnScenePlay();
					else if (m_SceneState != SceneState::Simulate)
						OnSceneStop();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip(m_SceneState == SceneState::Edit ? "Play" : "Stop");
					ImGui::PopStyleVar(2);
				}

				if (toolbarButton(EditorResources::SimulateButton, Theme::Text))
				{
					AR_WARN_TAG("EditorLayer", "Wassup Mate?");
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip("Simulate");
					ImGui::PopStyleVar(2);
				}

				if (toolbarButton(EditorResources::PauseButton, Theme::Text))
				{
					if (m_SceneState == SceneState::Play)
						m_SceneState = SceneState::Pause;
					else if (m_SceneState == SceneState::Pause)
						m_SceneState = SceneState::Play;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.5f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 1.0f, 1.0f });
					ImGuiUtils::ToolTip(m_SceneState == SceneState::Edit ? "Resume" : "Pause");
					ImGui::PopStyleVar(2);
				}

				ImGui::PopStyleVar();
			}

			ImGui::Spring();
			ImGui::EndHorizontal();
			ImGui::Spring();
			ImGui::EndVertical();

			ImGui::End();
		}
	}

	void EditorLayer::ShowViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

		m_ViewportRect = ImGuiUtils::GetWindowRect();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 viewportOffset = ImGui::GetCursorPos();
		m_ViewportSize = ImGui::GetContentRegionAvail();

		m_ViewportWidth = (uint32_t)m_ViewportSize.x;
		m_ViewportHeight = (uint32_t)m_ViewportSize.y;
		m_EditorScene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		if (m_RuntimeScene)
			m_RuntimeScene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		m_EditorCamera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		m_ViewportRenderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		void* textureID = (void*)(uint64_t)m_ViewportRenderer->GetFinalPassImage()->GetTextureID();
		ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// NOTE: FOR DEBUGGING!
		// This displays the textures used in the editor if i ever want to display a texture pretty quick just change the textID
		// ImGui::Image((void*)(uint64_t)Renderer::GetBRDFLutTexture()->GetTextureID(), { m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		// ImGui::Image((void*)(uint64_t)m_Renderer2D->GetFinalImage()->GetTextureID(), { m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		m_AllowViewportCameraEvents = (ImGuiUtils::IsMouseInRectRegion(m_ViewportRect, true) && m_ViewportFocused) || m_StartedRightClickInViewport;

		ImGuiUtils::SetInputEnabled(!m_ViewportFocused || !m_ViewportHovered);

		DrawGizmosToolBar();
		DrawCentralBar();

		// Gizmos...
		if (m_ShowGizmos && m_ActiveScene != m_RuntimeScene)
		{
			DrawGizmos();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		m_PanelsLibrary->OnImGuiRender();

		ImGui::End(); // Docking window
	}

	// TODO: The culling/depth/blend state will be removed when switching to vulkan since it does not have this much global state!
	void EditorLayer::ShowSettingsUI()
	{
		static bool enableCulling = true;
		static bool enableBlending = true;
		static bool enableDepthTesting = true;
		static bool allowGizmoAxisFlip = true;

		static std::string cullOption = "Back";
		static std::string depthOption = "Less";
		static float TickDelta = 1.0f;

		static const char* blendDesc = "Blending is the technique that allows and implements the \"Transparency\" within objects.";
		static const char* cullingDesc = "Culling is used to specify to the renderer what face is not to be processed thus reducing processing time.";
		static const char* depthDesc = "Depth testing is what allows for 3D objects to appear on the screen via a depth buffer.";
		static const char* comparatorOptions[] =
		{
			// For padding
			"None",
			// Depth
			"Never", "Equal", "NotEqual", "Less", "LessOrEqual", "Greater", "GreaterOrEqual", "Always",
			// Blend
			"OneZero", "SrcAlphaOneMinusSrcAlpha", "ZeroSrcAlpha",
			// Cull
			"Front", "Back", "FrontAndBack"
		};

		ImGui::Begin("Settings", &m_ShowSettingsUI);

		ImGuiUtils::BeginPropertyGrid(2, false);

		if(ImGuiUtils::PropertySliderFloat("App Tick Delta", TickDelta, 0.0f, 5.0f, "%.3f", "Controls the delta that it takes to call the OnTick function"))
			Application::GetApp().SetTickDeltaTime(TickDelta);

		ImGuiUtils::PropertyBool("Show Gizmos", m_ShowGizmos);
		ImGuiUtils::PropertyBool("Show Grid", m_ViewportRenderer->GetOptions().ShowGrid);
		ImGuiUtils::PropertyBool("Show Bounding Boxes", m_ShowBoundingBoxes);
		ImGuiUtils::PropertyBool("Show Icons", m_ShowIcons);
		if (ImGuiUtils::PropertyBool("Allow gizmo flip", allowGizmoAxisFlip))
			ImGuizmo::AllowAxisFlip(allowGizmoAxisFlip);
		
		ImGuiUtils::EndPropertyGrid();

		ImGui::PushID("RendererSettingsUI");
		if (ImGuiUtils::PropertyGridHeader("Renderer", false))
		{
			ImGuiUtils::BeginPropertyGrid(2, false);

			static const char* environmentMapSizesAndSamples[] = { "128", "256", "512", "1024", "2048", "4096" };
			RendererConfig& rendererConfig = Renderer::GetConfig();
			uint32_t currentSize = (uint32_t)glm::log2((float)rendererConfig.EnvironmentMapResolution) - 7;
			if (ImGuiUtils::PropertyDropdown("EnvironmentMap Size", environmentMapSizesAndSamples, 6, (int*)&currentSize))
			{
				rendererConfig.EnvironmentMapResolution = (uint32_t)glm::pow(2, currentSize + 7);
			}

			uint32_t currentSamples = (uint32_t)glm::log2((float)rendererConfig.IrradianceMapComputeSamples) - 7;
			if (ImGuiUtils::PropertyDropdown("IrradianceMap Compute Samples", environmentMapSizesAndSamples, 6, (int*)&currentSamples))
			{
				rendererConfig.IrradianceMapComputeSamples = (uint32_t)glm::pow(2, currentSamples + 7);
			}

			ImGuiUtils::PropertyBool("Enable Blending", enableBlending, blendDesc);

			int currentOption = (int)Comparator::SrcAlphaOnceMinusSrcAlpha;
			if (ImGuiUtils::PropertyDropdown("Blending Function", comparatorOptions, 3, &currentOption, blendDesc))
			{
				RenderCommand::SetCapabilityFunction(Capability::Blending, (Comparator)currentOption);
			}

			ImGuiUtils::PropertyBool("Enable Culling", enableCulling, cullingDesc);

			currentOption = (int)Comparator::Back;
			if (ImGuiUtils::PropertyDropdown("Culling Function", comparatorOptions, 3, &currentOption))
			{
				RenderCommand::SetCapabilityFunction(Capability::Culling, (Comparator)currentOption);
			}

			ImGuiUtils::PropertyBool("Enable Depth Testing", enableDepthTesting, depthDesc);

			currentOption = (int)Comparator::Less;
			if (ImGuiUtils::PropertyDropdown("Depth Function", comparatorOptions, 8, &currentOption))
			{
				RenderCommand::SetCapabilityFunction(Capability::DepthTesting, (Comparator)currentOption);
			}

			ImGuiUtils::EndPropertyGrid();
			ImGui::TreePop();
		}
		else
			ImGuiUtils::ShiftCursorY(-(ImGui::GetStyle().ItemSpacing.y + 1.0f));
		ImGui::PopID();

		ImGui::End();
	}

	void EditorLayer::ShowCloseModalUI()
	{
		ImGui::OpenPopup("Exit?");

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Exit?", &m_ShowCloseModal, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextColored(ImVec4{ 0.7f, 0.4f, 0.3f, 1.0f }, "Are you sure...?");
			ImGui::Text("Make sure you saved all your files and \nscenes before exiting!\n\n");

			ImGui::Separator();

			if (ImGui::Button("Exit", ImVec2(120, 0)))
				Application::GetApp().Close();

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				m_ShowCloseModal = false;
			}

			ImGui::EndPopup();
		}
	}

	void EditorLayer::OnEntityDeleted(Entity entity)
	{
		SelectionManager::Deselect(SelectionContext::Scene, entity.GetUUID());
	}

#pragma endregion

	void EditorLayer::OnImGuiRender()
	{
		AR_PROFILE_FUNCTION();

		EnableDocking();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !m_StartedRightClickInViewport))
		{
			ImGui::FocusWindow(GImGui->HoveredWindow);
			Input::SetCursorMode(CursorMode::Normal);
		}

		ShowMenuBarItems();

		// THIS IS FOR REFERENCE ONLY!
		// ImGui::ShowDemoWindow();

		if (m_ShowDearImGuiMetricsWindow)
			ImGui::ShowMetricsWindow(&m_ShowDearImGuiMetricsWindow);

		if (m_ShowDearImGuiStackToolWindow)
			ImGui::ShowStackToolWindow(&m_ShowDearImGuiStackToolWindow);

#ifdef AURORA_DEBUG
		if (m_ShowDearImGuiDebugLogWindow)
			ImGui::ShowDebugLogWindow(&m_ShowDearImGuiDebugLogWindow);
#endif

		if(m_ShowRendererVendorInfo)
			ShowRendererVendorInfoUI();

		if (m_ShowRenderStatsUI)
			ShowRendererStatsUI();

		if (m_ShowMaterialsPanel)
			ShowMaterialsPanel();

		if (m_ShowPerformance)
			ShowPerformanceUI();

		if (m_ShowScreenshotPanel)
			ShowScreenshotPanel();

		if (m_ShowEditingPanel)
			ShowEditPanelUI();

		if (m_ShowEditorCameraHelpUI)
			ShowEditorCameraHelpUI();

		if (m_ShowSettingsUI)
			ShowSettingsUI();

		if (m_ShowCloseModal)
			ShowCloseModalUI();

		ShowViewport();
	}

}