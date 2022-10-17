#include "EditorLayer.h"

#include "AssetManager/AssetManager.h"

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

		m_EditorScene = Scene::Create("Editor Scene");
		m_ActiveScene = m_EditorScene;
		m_ViewportRenderer = SceneRenderer::Create(m_ActiveScene);
		SetContextForSceneHeirarchyPanel(m_ActiveScene);
		m_Renderer2D = m_ViewportRenderer->GetRenderer2D();

		m_ViewportRenderer->SetLineWidth(m_LineWidth);

		// TODO: Temporary untill Projects are a thing!
		AssetManager::Init();
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
		if(m_AllowViewportCameraEvents)
			m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
		dispatcher.Dispatch<WindowPathDropEvent>([this](WindowPathDropEvent& e) { return OnPathDrop(e); });
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (GImGui->ActiveId == 0)
		{
			bool control = Input::IsKeyDown(AR_KEY_LEFT_CONTROL) || Input::IsKeyDown(AR_KEY_RIGHT_CONTROL);
			bool shift = Input::IsKeyDown(AR_KEY_LEFT_SHIFT) || Input::IsKeyDown(AR_KEY_RIGHT_SHIFT);
			bool alt = Input::IsKeyDown(AR_KEY_LEFT_ALT) || Input::IsKeyDown(AR_KEY_RIGHT_ALT);
			bool mousePressed = Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_LEFT) || Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_RIGHT) || Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_MIDDLE);
			bool inRuntime = m_ActiveScene == m_RuntimeScene;

			bool isSomethingSelected = m_SelectionContext ? true : false;

			switch (e.GetKeyCode())
			{
				// New Scene
				case KeyCode::N:
			    {
			    	if (control)
			    		NewScene();
			    
			    	break;
			    }
			    
				// Open Scene
				case KeyCode::O:
			    {
			    	if (control)
			    		OpenScene();
			    
			    	break;
			    }
			    
				// Save Scene
				case KeyCode::S:
			    {
			    	if (control)
			    	{
			    		if (shift)
			    			SaveSceneAs();
			    		else
			    			SaveScene();
			    	}
			    
			    	break;
			    }
			    
				// Duplicate entity
				case KeyCode::D:
			    {
			    	if (control && isSomethingSelected && !mousePressed)
			    	{
			    		m_SelectionContext = m_ActiveScene->CopyEntity(m_SelectionContext);
			    	}
			    
			    	break;
			    }
			    
				// Delete entity
				case KeyCode::Delete:
			    {
			    	if (isSomethingSelected)
			    	{
			    		m_ActiveScene->DestroyEntity(m_SelectionContext);
			    		m_SelectionContext = {};
			    	}
			    
			    	break;
			    }
			    
			    // Gizmos
				case KeyCode::Q:
			    {
					if (inRuntime)
						break;

			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = -1;
			    
			    	break;
			    }
			    
				case KeyCode::W:
			    {
					if (inRuntime)
						break;

			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			    
			    	break;
			    }
			    
				case KeyCode::E:
			    {
					if (inRuntime)
						break;

			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			    
			    	break;
			    }
			    
				case KeyCode::R:
			    {
					if (inRuntime)
						break;

			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::SCALE;
			    
			    	break;
			    }

				// Reset Editor Camera
				case KeyCode::C:
				{
					if (alt)
					{
						m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f);
					}

					break;
				}

				// Reset focal point
				case KeyCode::F:
				{
					if (alt)
					{
						m_EditorCamera.Focus({ 0.0f, 0.0f, 0.0f });
					}

					break;
				}
			}
		}

		return true;
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

				Ref<StaticMesh> staticMesh = smc.StaticMesh;
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
								AR_WARN("INTERSECTION: {0}, t = {1}", subMesh.NodeName, t);
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
				m_SelectionContext = {};

			if (!selectionData.empty())
			{
				Entity entityy = selectionData.front().Entity;
				m_SelectionContext = entityy;
			}
		}

		return false;
	}

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

#pragma region SceneHierarchyPanel

	void EditorLayer::SetContextForSceneHeirarchyPanel(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
		// This is to prevent a bug from appearing and that is when an entity is selected and then we try to create a new scene
		// however this might lead to bugs later if, IF we ever get to the point were we have multiple scene panels open at the same time and we want
		// the selection context to go with every scene it is in
	}

	void EditorLayer::DrawEntityCreatePopupMenu(Entity entiy)
	{
		if (!ImGui::BeginMenu("Create"))
			return;

		Entity newEntity;

		if (ImGui::MenuItem("Empty Entity"))
		{
			newEntity = m_Context->CreateEntity("Empty Entity");
			m_SelectionContext = newEntity;
		}

		if (ImGui::MenuItem("Camera"))
		{
			newEntity = m_Context->CreateEntity("Camera");
			newEntity.AddComponent<CameraComponent>();
			m_SelectionContext = newEntity;
		}

		ImGui::EndMenu();
	}

	void EditorLayer::DrawEntityNode(Entity entity, const std::string& searchedString)
	{
		// All entities have a tag component by default
		const std::string& tag = entity.GetComponent<TagComponent>().Tag;

		if (!ImGuiUtils::IsMatchingSearch(tag, searchedString, false, true))
			return;

		std::string strID = fmt::format("{}{}", tag, entity.GetUUID());

		constexpr float edgeOffset = 4.0f;
		constexpr float rowHeight = 21.0f;

		static bool isHovered = false;
		static bool isHeld = false;
		static bool isSelected = false;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;
		ImGui::TableNextRow(0, rowHeight);
		window->DC.CurrLineTextBaseOffset = 3.0f;
		ImGui::TableNextColumn();

		// This is the min and max of the rect of the WHOLE row with all its columns and not just the row min/max of the first column!
		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x, rowAreaMin.y + rowHeight + 4.0f };
		
		// This creates a rect that spans the area of the whole row and thus allows the detection of mouse hovered actions in
		// consecutive rows which is not exactly possible if I dont push and pop a clip rect
		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);

		constexpr ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_AllowItemOverlap
			| ImGuiButtonFlags_PressedOnClickRelease
			| ImGuiButtonFlags_MouseButtonLeft
			| ImGuiButtonFlags_MouseButtonRight;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect{ rowAreaMin, rowAreaMax }, ImGui::GetID(strID.c_str()), &isHovered, &isHeld, buttonFlags);
		bool wasRowRightClicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

		ImGui::SetItemAllowOverlap();

		ImGui::PopClipRect();

		// Row Coloring...
		auto fillRowWithColour = [](const ImColor& color)
		{
			for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color, column);
		};

		isSelected = m_SelectionContext == entity;
		ImGuiTreeNodeFlags treeFlags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
		treeFlags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;

		if (isSelected)
		{
			fillRowWithColour(ImColor(236, 158, 36, 150));
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextBrighter);
		}
		else if (isHovered)
		{
			fillRowWithColour(IM_COL32(236, 158, 36, 200));
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::Background);
		}

		// Taken from ImGui TreeNodeBehavior arrow hit calculations and slightly modified!
		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec2 labelSize = ImGui::CalcTextSize(strID.c_str());
		const ImVec2 padding = (treeFlags & ImGuiTreeNodeFlags_FramePadding) ? style.FramePadding 
			: ImVec2{ style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y)};

		const float text_offset_x = GImGui->FontSize + padding.x; // Collapser arrow width + Spacing
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset); // Latch before ItemSize changes it
		const float textWidth = GImGui->FontSize + (labelSize.x > 0.0f ? labelSize.x + padding.x * 2 : 0.0f); // Include collapser
		
		ImVec2 text_pos = { window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y };
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (GImGui->FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (GImGui->IO.MousePos.x >= arrow_hit_x1 && GImGui->IO.MousePos.x < arrow_hit_x2);

		bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(strID.c_str()));

		if (is_mouse_x_over_arrow && isRowClicked)
			ImGui::SetNextItemOpen(!previousState);

		ImGui::PushStyleColor(ImGuiCol_Header, 0);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);
		const bool opened = ImGui::TreeNodeEx(strID.c_str(), treeFlags, tag.c_str());
		ImGui::PopStyleColor(isSelected || isHovered ? 4 : 3);

		bool inSceneHierarchyTable = ImGuiUtils::IsMouseInRectRegion(m_SceneHierarchyTableRect, false);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& ImGuiUtils::IsMouseInRectRegion({ rowAreaMin, rowAreaMax }, false)
			&& inSceneHierarchyTable)
		{
			m_SelectionContext = entity;
		}

		bool mouseClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::TableGetRowIndex())
			|| (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && Input::IsKeyDown(KeyCode::LeftShift) 
			&& inSceneHierarchyTable);
		if (mouseClicked)
			m_SelectionContext = {};
		
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Theme::AccentDimmed);
		ImGui::PushStyleColor(ImGuiCol_Text, Theme::Text);
		ImGui::PushStyleColor(ImGuiCol_Header, Theme::GroupHeader);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, Theme::GroupHeader);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Theme::GroupHeader);
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8, 8 });

		std::string rightClickPopupID = fmt::format("{0}-ContextPopupMenu", strID);

		bool deleteEntity = false;
		if (ImGui::BeginPopupContextItem(rightClickPopupID.c_str()))
		{
			DrawEntityCreatePopupMenu(entity);

			if (ImGui::MenuItem("Delete"))
				deleteEntity = true;

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(5);

		// TODO: Add other identifiers when there actually are other identifiers such as prefabs when these are a thing
		ImGui::TableNextColumn();
		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextBrighter);
		else if(isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::Background);
		ImGui::Text("Entity");
		if (isHovered || isSelected)
			ImGui::PopStyleColor();

		// Mouse Click events also for this column
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& ImGuiUtils::IsMouseInRectRegion({ rowAreaMin, rowAreaMax }, false)
			&& inSceneHierarchyTable)
		{
			m_SelectionContext = entity;
		}

		if (isRowClicked && wasRowRightClicked)
		{
			ImGui::OpenPopup(rightClickPopupID.c_str());
		}

		if (opened)
		{
			// TODO: here we should display the child entites of a parent entity once parenting entites is a thing!
			ImGui::TableNextRow(0, rowHeight);
			ImGui::TableNextColumn();

			ImGuiUtils::ShiftCursorX(10.0f);
			ImGui::TextColored(ImVec4{ 0.4f, 0.8f, 0.4f, 1.0f }, "WIP! Until parenting entities is a thing!");

			ImGui::TreePop();
		}

		if(deleteEntity)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	void EditorLayer::ShowSceneHierarchyPanel()
	{
		ImGuiFontsLibrary& fontsLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene Hierarchy", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);

		static bool enableSorting = false;
		constexpr float edgeOffset = 4.0f;

		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		fontsLib.PushTemporaryFont("RobotoBold");
		ImGui::TextColored(ImVec4{ 0.925f, 0.619f, 0.141f, 0.888f }, "Scene:");

		ImGui::SameLine();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f - 30.0f);

		std::string& sceneName = m_EditorScene->GetName();
		// There should not be a scene name that is more than 128 characters long LOL
		char buffer[128];
		strcpy_s(buffer, sizeof(buffer), sceneName.c_str());
		ImGuiUtils::ShiftCursorY(-2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		if(ImGui::InputTextWithHint(ImGuiUtils::GenerateID(), "Enter scene name...", buffer, sizeof(buffer)))
		{
			sceneName = std::string(buffer);
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		fontsLib.PopTemporaryFont();

		ImGuiUtils::DrawItemActivityOutline(3.0f, false, Theme::Accent);

		ImGui::SameLine();

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f; // From ImGui source
		ImGuiUtils::ShiftCursorX(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f - 26.0f);
		void* textureID = (void*)(uint64_t)EditorResources::GearIcon->GetTextureID();
		if (ImGui::ImageButton(textureID, ImVec2{ lineHeight * 0.75f, lineHeight * 0.75f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }))
			ImGui::OpenPopup("SceneHierarchySettings");

		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 2.5f });
		if (ImGui::BeginPopup("SceneHierarchySettings"))
		{
			ImGui::TextColored(ImVec4{ 0.3f, 0.9f, 0.3f, 1.0f }, "Scene Panel Settings");

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 150.0f);

			ImGui::Text("Show Properties: ");

			ImGui::NextColumn();

			if (ImGui::Checkbox("##ShowProperties", &m_ShowPropertiesPanel))
				ImGui::CloseCurrentPopup();

			ImGui::NextColumn();

			ImGui::Text("Enable Sorting: ");

			ImGui::NextColumn();

			if (ImGui::Checkbox("##EnableSorting", &enableSorting))
				ImGui::CloseCurrentPopup();

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::TextColored(ImVec4{ 0.3f, 0.9f, 0.3f, 1.0f }, "Scene Info:");
			ImGui::Text("Number of entites in scene: %d", m_Context->GetAllEntitiesWith<IDComponent>().size());

			ImGui::Separator();

			ImGui::TextColored(ImVec4{ 0.3f, 0.9f, 0.3f, 1.0f }, "Help");
			ImGui::Text("To deselect an entity: Shift + LeftClick");

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);

		ImGuiUtils::ShiftCursorY(-edgeOffset * 2.0f);
		// X shifting resets by itself via imgui internally when going to a new line!

		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f);

		static std::string searchString;
		ImGuiUtils::SearchBox(searchString);

		ImGui::Spacing();
		ImGui::Spacing();

		// Table
		{
			const ImU32 colRowAlt = ImGuiUtils::ColorWithMultiplierValue(Theme::BackgroundDark, 1.3f);
			ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, colRowAlt);

			// This sets the background of the table since a table is considered as a child window
			ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BackgroundDark);

			constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX
				| ImGuiTableFlags_Resizable
				//| ImGuiTableFlags_Reorderable
				| ImGuiTableFlags_ScrollY
				| ImGuiTableFlags_RowBg;

			if (ImGui::BeginTable("SceneHierarchy-EntityListTable", 2, tableFlags, ImGui::GetContentRegionAvail()))
			{
				ImGui::TableSetupColumn("Label");
				ImGui::TableSetupColumn("Type");

				m_SceneHierarchyTableRect = ImGui::GetCurrentTable()->InnerRect;

				// Setup the header for the table...
				ImU32 headerColor = ImGuiUtils::ColorWithMultiplierValue(Theme::AccentDimmed, 1.2f);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);
				ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);

				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
				for (int i = 0; i < ImGui::TableGetColumnCount(); i++)
				{
					ImGui::TableSetColumnIndex(i);
					const char* columnName = ImGui::TableGetColumnName();

					ImGui::PushID(i);

					ImGuiUtils::ShiftCursor(edgeOffset * 3.0f, 0.0f);

					ImGui::TableHeader(columnName, 0, Theme::Background);

					ImGuiUtils::ShiftCursor(-edgeOffset * 3.0f, 0.0f);

					ImGui::PopID();
				}
				ImGui::SetCursorPosX(ImGui::GetCurrentTable()->OuterRect.Min.x);

				if(enableSorting)
				{
					for (auto entity : m_Context->GetAllEntitiesWith<IDComponent>())
					{
						m_SortedEntities.emplace_back(Entity{ entity, m_Context.raw() });
					}

					std::sort(m_SortedEntities.begin(), m_SortedEntities.end(), [](Entity a, Entity b)
					{
						return a.GetComponent<TagComponent>().Tag < b.GetComponent<TagComponent>().Tag;
					});

					for (Entity ent : m_SortedEntities)
					{
						DrawEntityNode(ent, searchString);
					}

					m_SortedEntities.clear();
				}
				else
				{
					for (auto entity : m_Context->GetAllEntitiesWith<IDComponent>())
					{
						DrawEntityNode({ entity, m_Context.raw() }, searchString);
					}
				}

				ImGui::PopStyleColor(2);

				ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8, 8 });
				if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
				{
					DrawEntityCreatePopupMenu({});
					ImGui::EndPopup();
				}
				ImGui::PopStyleVar(2);

				ImGui::EndTable();
			}

			ImGui::PopStyleColor(2);
		}

		if (m_ShowPropertiesPanel)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8, 8 });
			ImGui::Begin("Properties", (bool*)0, ImGuiWindowFlags_NoMove);

			if (m_SelectionContext)
			{
				DrawComponents(m_SelectionContext);
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	template<typename T, typename UIFunction, typename ResetFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, ResetFunction resetFunction, int texID = -1)
	{
		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			ImGui::PushID((void*)typeid(T).hash_code());

			T& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.0f, 4.0f });
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f; // From ImGui source

			// TODO: Add component frame icons
			//bool open = ImGui::TreeNodeWithIcon(EditorResources::ComponentIcon, (void*)typeid(T).hash_code(), treeNodeFlags, ImColor(255, 255, 255, 255), name.c_str());
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				void* textureID = (void*)(uint64_t)EditorResources::CloseIcon->GetTextureID();
				if (ImGui::ImageButton(textureID, ImVec2{ lineHeight - 9.0f, lineHeight - 6.0f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }))
				{
					ImGui::OpenPopup("ComponentSettings"); // This is also just an id and not a tag that will be rendered
				}

				ImGui::SameLine(contentRegionAvail.x - 1.5f * lineHeight);
				
				textureID = (void*)(uint64_t)EditorResources::ResetIcon->GetTextureID();
				if (ImGui::ImageButton(textureID, ImVec2{ lineHeight - 8.0f, lineHeight - 6.0f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }))
				{
					resetFunction(component);
				}

				if (ImGui::IsItemHovered())
					ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "Reset");
			}
			else
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);

				void* textureID = (void*)(uint64_t)EditorResources::ResetIcon->GetTextureID();
				if (ImGui::ImageButton(textureID, ImVec2{ lineHeight - 8.0f, lineHeight - 6.0f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }))
				{
					resetFunction(component);
				}

				if (ImGui::IsItemHovered())
					ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "Reset");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f });
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				ImGui::PopStyleColor();

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();

			ImGui::PopID();
		}
	}

	void EditorLayer::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float colomnWidth, float min, float max, float stepValue)
	{
		ImGuiLayer* layer = Application::GetApp().GetImGuiLayer();

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, colomnWidth);
		ImGui::Text(label.c_str());

		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		const ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		layer->m_FontsLibrary.PushTemporaryFont("MochiyPopOne");
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		layer->m_FontsLibrary.PushTemporaryFont("MochiyPopOne");
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		layer->m_FontsLibrary.PushTemporaryFont("MochiyPopOne");
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();

		ImGui::PopStyleVar(2);
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template<typename Component, bool TAddSeparator = false>
	static void DrawPopUpMenuEntry(std::string_view compName, Entity entity, Entity& selectionContext)
	{
		if (!entity.HasComponent<Component>())
		{
			if (ImGui::MenuItem(compName.data()))
			{
				selectionContext.AddComponent<Component>();
				ImGui::CloseCurrentPopup();
			}

			if constexpr (TAddSeparator)
			{
				ImGui::Separator();
			}
		}
	}

	void EditorLayer::DrawComponents(Entity entity)
	{
		ImGuiFontsLibrary& fontLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;
		std::string& tag = entity.GetComponent<TagComponent>().Tag;

		// Size of buffer is 128 because there should not be an entity called more than 128 letters like BRUH
		char buffer[128];
		strcpy_s(buffer, sizeof(buffer), tag.c_str());

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		fontLib.PushTemporaryFont("RobotoBold");
		if (ImGui::InputTextWithHint("##Tag", "Change entity name...", buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}
		fontLib.PopTemporaryFont();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		ImGuiUtils::DrawItemActivityOutline(3.0f, false, Theme::Accent);

		if (ImGui::IsItemHovered())
			ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "ID: %#010x", entity.GetUUID());

		ImGui::SameLine();

		float lineHeight = ImGui::GetItemRectSize().y; // Get Height of last item

		//ImGuiUtils::ShiftCursorX(-5);
		if (ImGui::Button("Add Component", ImVec2{ ImGui::GetContentRegionAvail().x, lineHeight }))
			ImGui::OpenPopup("AddComponentsPopUp"); // AddComponentPopUp here acts as an ID for imgui to be used in the next step

		if (ImGui::BeginPopup("AddComponentsPopUp"))
		{
			DrawPopUpMenuEntry<CameraComponent, true>("Camera", entity, m_SelectionContext);
			DrawPopUpMenuEntry<SpriteRendererComponent>("Sprite Renderer", entity, m_SelectionContext);
			DrawPopUpMenuEntry<CircleRendererComponent>("Circle Renderer", entity, m_SelectionContext);
			DrawPopUpMenuEntry<StaticMeshComponent, true>("StaticMesh", entity, m_SelectionContext);
			DrawPopUpMenuEntry<DirectionalLightComponent>("DirectionalLight", entity, m_SelectionContext);
			DrawPopUpMenuEntry<SkyLightComponent, true>("SkyLight", entity, m_SelectionContext);
			DrawPopUpMenuEntry<TextComponent>("Text", entity, m_SelectionContext);

			ImGui::EndPopup();
		}

		DrawComponent<TransformComponent>("Transform", entity, [this](TransformComponent& component)
		{
			DrawVec3Control("Translation", component.Translation);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVec3Control("Rotation", rotation);
			component.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component.Scale, 1.0f, 100.0f, 0.0f, 1000.0f, 0.025f);
		},
		[](TransformComponent& component) // Reset function
		{
			component.Translation = glm::vec3(0.0f);
			component.Rotation = glm::vec3(0.0f);
			component.Scale = glm::vec3(1.0f);
		});

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& component)
		{
			SceneCamera& camera = component.Camera;

			ImGuiUtils::BeginPropertyGrid();

			ImGuiUtils::PropertyBool("Primary", component.Primary, "Sets whether this is the primary camera to render with if there are multiple of them");

			static const char* projectionTypeString[] = { "Perspective", "Orthographic" };
			int currentProjectionType = (int)camera.GetProjectionType();
			if (ImGuiUtils::PropertyDropdown("Projection", projectionTypeString, 2, &currentProjectionType, "Set the type of projection for camera"))
			{
				camera.SetProjectionType((SceneCamera::ProjectionType)currentProjectionType);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float verticalFOV = camera.GetDegPerspectiveVerticalFOV();
				if (ImGuiUtils::PropertyFloat("FOV", verticalFOV, 0.1f, 0.0f, 120.0f, "Field of view"))
					camera.SetDegPerspectiveVerticalFOV(verticalFOV);

				float persNear = camera.GetPerspectiveNearClip();
				if (ImGuiUtils::PropertyFloat("Near", persNear, 0.1f, 0.0f, 0.0f, "Set near clip of camera"))
					camera.SetPerspectiveNearClip(persNear);

				float persFar = camera.GetPerspectiveFarClip();
				if (ImGuiUtils::PropertyFloat("Far", persFar, 0.1f, 0.0f, 0.0f, "Set far clip of camera"))
					camera.SetPerspectiveFarClip(persFar);
			}
			else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGuiUtils::PropertyFloat("Size", orthoSize, 0.1f, 0.0f, 1000.0f, "Set size of camera"))
					camera.SetOrthographicSize(orthoSize);

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGuiUtils::PropertyFloat("Near", orthoNear, 0.1f, 0.0f, 0.0f, "Set near clip of camera"))
					camera.SetOrthographicNearClip(orthoNear);

				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGuiUtils::PropertyFloat("Far", orthoFar, 0.1f, 0.0f, 0.0f, "Set far clip of camera"))
					camera.SetOrthographicFarClip(orthoFar);
			}

			ImGuiUtils::EndPropertyGrid();
		},
		[](CameraComponent& component) // Reset Function
		{
			auto& camera = component.Camera;
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				camera.SetDegPerspectiveVerticalFOV(45.0f);
				camera.SetPerspectiveNearClip(0.01f);
				camera.SetPerspectiveFarClip(1000.0f);
			}
			else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				camera.SetOrthographicSize(10.0f);
				camera.SetOrthographicNearClip(-1.0f);
				camera.SetOrthographicFarClip(1.0f);
			}
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGuiUtils::ColorEdit4Control("Color", component.Color);
			// TODO: Add textue selection drag drop API
			ImGuiUtils::PropertyFloat("Tiling Factor", component.TilingFactor, 0.1f, 1.0f, 10.0f);

			ImGuiUtils::EndPropertyGrid();
		},
		[](SpriteRendererComponent& component) // Reset Function
		{
			component.Color = glm::vec4(1.0f);
			component.TilingFactor = 1.0f;
		});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGuiUtils::ColorEdit4Control("Color", component.Color);
			ImGuiUtils::PropertyFloat("Thickness", component.Thickness, 0.01f, 0.0f, 1.0f);

			ImGuiUtils::EndPropertyGrid();
		},
		[](CircleRendererComponent& component)
		{
			component.Color = glm::vec4(1.0f);
			component.Thickness = 1.0f;
		});

		static std::string environmentMapName = "Aurora Default";

		DrawComponent<SkyLightComponent>("SkyLight", entity, [](SkyLightComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGui::Text("Environment");

			ImGui::NextColumn();

			// TODO: Change to accept drag drop
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushStyleColor(ImGuiCol_Button, Theme::PropertyField);
			if (ImGui::Button(environmentMapName.c_str(), { width, 25.0f }))
			{
				std::filesystem::path p = Utils::WindowsFileDialogs::OpenFileDialog("HDR Image (*.hdr)\0*.hdr\0");
				environmentMapName = p.filename().string();

				if (environmentMapName.size())
				{
					Ref<Asset> asset = AssetManager::GetAsset<Environment>(p);
					if (asset->IsValid())
						component.SceneEnvironment = asset->Handle;
				}
				else
					environmentMapName = "Aurora Default";
			}
			ImGui::PopStyleColor();			

			ImGui::NextColumn();

			ImGuiUtils::PropertyFloat("LOD", component.Level, 0.05f, 0.0f, 10.0f);
			ImGuiUtils::PropertyFloat("Intensity", component.Intensity, 0.05f, 0.01f, 10.0f);
			ImGuiUtils::PropertyBool("Dynamic Sky", component.DynamicSky, "Generates a gpu generated skybox");
			ImGuiUtils::PropertyFloat("Turbidity", component.TurbidityAzimuthInclination.x, 0.05f, 2.0f, 20.0f, "How turbid the sky is");
			ImGuiUtils::PropertyFloat("Azimuth", component.TurbidityAzimuthInclination.y, 0.05f, 0.0f, 20.0f);
			ImGuiUtils::PropertyFloat("Inclination", component.TurbidityAzimuthInclination.z, 0.05f, 0.0f, 20.0f, "How inclined the sun is");

			ImGuiUtils::EndPropertyGrid();
		},
		[](SkyLightComponent& component) // Reset Function
		{
			environmentMapName = "Aurora Default";
			component.SceneEnvironment = 0;
			component.Level = 0.5f;
			component.Intensity = 1.0f;
			component.DynamicSky = false;
			component.TurbidityAzimuthInclination = { 2.0f, 0.0f, 0.0f };
		});

		DrawComponent<StaticMeshComponent>("StaticMesh", entity, [](StaticMeshComponent& component)
		{
			if (ImGui::Button("Static Mesh Path"))
			{
				std::filesystem::path path = Utils::WindowsFileDialogs::OpenFileDialog("StaticMesh");
				Ref<MeshSource> meshSource = MeshSource::Create(path);
				component.StaticMesh = StaticMesh::Create(meshSource);
			}
		},
		[](StaticMeshComponent& component)
		{
		});

		DrawComponent<DirectionalLightComponent>("DirectionalLight", entity, [](DirectionalLightComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGuiUtils::ColorEdit3Control("Radiance", component.Radiance);
			ImGuiUtils::PropertyFloat("Intensity", component.Intensity, 0.01f, 0.0f, 10.0f);

			ImGuiUtils::EndPropertyGrid();
		},
		[](DirectionalLightComponent& component)
		{
			component.Radiance = { 1.0f, 1.0f, 1.0f };
			component.Intensity = 1.0f;
		});
		
		static std::string textFileName = "Aurora Default";

		DrawComponent<TextComponent>("Text", entity, [](TextComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGui::Text("Font");

			ImGui::NextColumn();

			// TODO: Change to accept drag drop
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushStyleColor(ImGuiCol_Button, Theme::PropertyField);
			if (ImGui::Button(textFileName.c_str(), { width, 25.0f }))
			{
				std::filesystem::path p = Utils::WindowsFileDialogs::OpenFileDialog("Font File (*.ttf)\0*.ttf\0");
				textFileName = p.filename().string();

				if (textFileName.size())
				{
					Ref<Asset> asset = AssetManager::GetAsset<Font>(p);
					if (asset->IsValid())
						component.FontHandle = asset->Handle;
				}
				else
					textFileName = "Aurora Default";
			}
			ImGui::PopStyleColor();

			ImGui::NextColumn();

			if (ImGuiUtils::MultiLineText("Text", component.TextString))
			{
				component.TextHash = std::hash<std::string>()(component.TextString);
			}
			
			ImGui::Separator();

			ImGuiUtils::ColorEdit4Control("Color", component.Color);
			ImGuiUtils::PropertyFloat("Line Spacing", component.LineSpacing, 0.05f, 0.0f, 30.0f);
			ImGuiUtils::PropertyFloat("Max Width", component.MaxWidth, 0.01f, 0.0f, 30.0f);
			ImGuiUtils::PropertyFloat("Kerning", component.Kerning, 0.01f, 0.0f, 30.0f);

			ImGuiUtils::EndPropertyGrid();
		},
		[](TextComponent& component)
		{
			component.TextString = "";
			component.TextHash = 0;
			component.FontHandle = Font::GetDefaultFont()->Handle;
			component.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
			component.LineSpacing = 0.0f;
			component.Kerning = 0.0f;
			component.MaxWidth = 10.0f;
			textFileName = "Aurora Default";
		});
	}

#pragma endregion

	// TODO: NEED TO CHANGE SOME STUFF HERE FOR LATER
#pragma region FileDialogs/Scene Helpers

	void EditorLayer::NewScene()
	{
		m_EditorScene->Clear();
		m_EditorScene = nullptr;
		m_ActiveScene = nullptr;

		m_EditorScene = Scene::Create("New Scene"); // Creating new scene
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		SetContextForSceneHeirarchyPanel(m_EditorScene);

		m_ActiveScene = m_EditorScene;

		m_EditorScenePath = std::filesystem::path();
		m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f);
	}

	void EditorLayer::OpenScene()
	{
		std::filesystem::path filepath = Utils::WindowsFileDialogs::OpenFileDialog("Aurora Scene (*.aurora)\0*.aurora\0");
		
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (path.extension().string() != ".aurora")
		{
			AR_WARN_TAG("EditorLayer", "Could not load '{0}' - not an Aurora scene file!", path.filename().string());
			
			return;
		}

		Ref<Scene> newScene = Scene::Create();
		SceneSerializer serializer(newScene);

		if (serializer.DeSerializeFromText(path.string()))
		{
			m_EditorScene = newScene;
			SetContextForSceneHeirarchyPanel(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
			m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		std::filesystem::path filepath = Utils::WindowsFileDialogs::SaveFileDialog("Aurora Scene (*.aurora)\0*.aurora\0");

		if (!filepath.empty())
		{
			SerializeScene(m_ActiveScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::SerializeScene(const Ref<Scene>& scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.SerializeToText(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		m_SelectionContext = {};

		m_SceneState = SceneState::Play;

		m_RuntimeScene = Scene::Create();
		m_EditorScene->CopyTo(m_RuntimeScene);
		m_RuntimeScene->OnRuntimeStart();
		m_ActiveScene = m_RuntimeScene;
	}

	void EditorLayer::OnSceneSimulate()
	{

	}

	void EditorLayer::OnSceneStop()
	{
		m_SelectionContext = {};

		m_RuntimeScene->OnRuntimeStop();

		m_SceneState = SceneState::Edit;
		Input::SetCursorMode(CursorMode::Normal);

		m_RuntimeScene = nullptr;
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

	void EditorLayer::ShowShadersPanel()
	{
		ImGui::Begin("Shaders", &m_ShowShadersPanel);

		constexpr float edgeOffset = 4.0f;
		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f);

		static std::string searchString;
		ImGuiUtils::SearchBox(searchString);

		ImGui::Spacing();
		ImGui::Spacing();

		constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		for (Ref<Shader> shader : Shader::s_AllShaders)
		{
			const std::string& shaderName = shader->GetName();

			if (!ImGuiUtils::IsMatchingSearch(shaderName, searchString))
				continue;

			constexpr float framePaddingX = 5.0f;
			constexpr float framePaddingY = 5.0f; // affects height of the header

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });
			std::string name = fmt::format("{0}", shaderName);
			const bool opened = ImGui::TreeNodeEx(name.c_str(), treeNodeFlags);
			ImGui::PopStyleVar(2);

			ImGuiUtils::ShiftCursorY(-4.0f);

			if(opened)
			{
				ImGui::Text("Type: %s", shader->GetTypeString().c_str());
				ImGui::Text("Path: %s", shader->GetFilePath().string().c_str());

				uint32_t lastTimeMod = shader->GetLastTimeModified();
				if (lastTimeMod <= shader->GetCompileTimeThreshold())
					ImGui::TextColored(ImVec4{ 0.8f, 0.2f, 0.3f, 1.0f }, "Last time modified: %u minutes", lastTimeMod);
				else
					ImGui::Text("Last time modified: %u minutes", lastTimeMod);

				if (ImGui::Button("Reload"))
					shader->Reload();

				ImGui::TreePop();
			}
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

		std::string finalName = "SandboxProject/Screenshots/" + m_Context->GetName() + "_" + timeString + ".png";
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
				if (ImGui::MenuItem("Scene Hierarchy", nullptr, m_ShowSceneHierarchyPanel))
					m_ShowSceneHierarchyPanel = !m_ShowSceneHierarchyPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Shaders", nullptr, m_ShowShadersPanel))
					m_ShowShadersPanel = !m_ShowShadersPanel;

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

			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Settings...", nullptr, m_ShowSettingsUI))
					m_ShowSettingsUI = !m_ShowSettingsUI;

				ImGui::Separator();

				if (ImGui::MenuItem("Exit...", nullptr, m_ShowCloseModal))
					m_ShowCloseModal = !m_ShowCloseModal;

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

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Editor Camera", nullptr, m_ShowEditorCameraHelpUI))
					m_ShowEditorCameraHelpUI = !m_ShowEditorCameraHelpUI;

				ImGui::EndMenu();
			}

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

	void EditorLayer::ManipulateGizmos()
	{
		// Camera
		// Editor Camera
		const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
		const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();

		// Runtime Camera
		// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
		// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		// const glm::mat4& cameraProjection = camera.GetProjection();
		// glm::mat4 cameraView2 = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

		// Entity transform
		auto& tc = m_SelectionContext.GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		// Snapping
		bool snap = Input::IsKeyDown(KeyCode::LeftControl);

		const float snapValue = GetSnapValue();

		const float snapValues[3] = { snapValue, snapValue, snapValue };

		if(ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
			nullptr, snap ? snapValues : nullptr) && !Input::IsKeyDown(KeyCode::LeftAlt))
		{
			glm::vec3 translation(0.0f), rotation(0.0f), scale(0.0f);
			Math::DecomposeTransform(transform, translation, rotation, scale);

			switch (m_GizmoType)
			{
			    case ImGuizmo::TRANSLATE:
			    {
					tc.Translation = translation;
					break;
			    }
				case ImGuizmo::ROTATE:
				{
					tc.Rotation += rotation - tc.Rotation;
					break;
				}
				case ImGuizmo::SCALE:
				{
					tc.Scale = scale;
					break;
				}
			}
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

		// Gizmos...
		if (m_SelectionContext != Entity::nullEntity && m_GizmoType != -1 && m_ActiveScene != m_RuntimeScene)
		{
			float windowWidth = m_ViewportRect.Max.x - m_ViewportRect.Min.x;
			float windowHeight = m_ViewportRect.Max.y - m_ViewportRect.Min.y;

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportRect.Min.x, m_ViewportRect.Min.y, windowWidth, windowHeight);

			ManipulateGizmos();
		}

		// Viewport Scene Tools
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

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
			ImGui::PopStyleVar(4);
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End(); // Docking window
	}

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

		ImGuiUtils::PropertyBool("Show Grid", m_ViewportRenderer->GetOptions().ShowGrid);
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

		if (m_ShowSceneHierarchyPanel)
			ShowSceneHierarchyPanel();

		if(m_ShowRendererVendorInfo)
			ShowRendererVendorInfoUI();

		if (m_ShowRenderStatsUI)
			ShowRendererStatsUI();

		if (m_ShowShadersPanel)
			ShowShadersPanel();

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