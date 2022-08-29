#include "EditorLayer.h"

#include "ImGui/ImGuiUtils.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

#include <math.h>

namespace Aurora {

#pragma region EditorLayerMainMethods

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_EditorCamera(EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f))
	{
	}

	static glm::vec3 s_MaterialAlbedoColor(1.0f);

	void EditorLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		EditorResources::Init();

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = Scene::Create("Editor Scene");
		m_ActiveScene = m_EditorScene;
		SetContextForSceneHeirarchyPanel(m_ActiveScene);

		// TODO: Default open scene for now...!
		OpenScene("Resources/scenes/TestingSearchBox.aurora");

		class CameraScript : public ScriptableEntity
		{
			virtual void OnUpdate(TimeStep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				const float speed = 5.0f;

				if (Input::IsKeyPressed(AR_KEY_W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(AR_KEY_S))
					translation.y -= speed * ts;
				if (Input::IsKeyPressed(AR_KEY_A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(AR_KEY_D))
					translation.x += speed * ts;
			}
		};
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

		// Framebuffer resizing... This stops the blacked out frames we would get when resizing...
		// TODO: This should not be handled here...
		FramebufferSpecification spec = m_Framebuffer->GetSpecification();
		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_EditorCamera.SetActive(m_AllowViewportCameraEvents);
		m_EditorCamera.OnUpdate(ts);

		Renderer3D::ResetStats();

		m_Framebuffer->Bind();
		RenderCommand::SetClearColor(glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f }); // TODO: Framebuffers should contain the clear colors and not called like that, and then maybe clear is called in begin scene
		RenderCommand::Clear();
		m_Framebuffer->ClearTextureAttachment(1, -1);

		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera, s_MaterialAlbedoColor);
		//m_ActiveScene->OnUpdateRuntime(ts);

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportRect.Min.x;
		my -= m_ViewportRect.Min.y;
		ImVec2 viewportSize = { m_ViewportRect.Max.x - m_ViewportRect.Min.x, m_ViewportRect.Max.y - m_ViewportRect.Min.y };
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if(ImGuiUtils::IsMouseInRectRegion(m_ViewportRect, false))
		{
			// This is a very slow operation, and itself adds about 1-2 milliseconds to our CPU Frame, however this is just for the editor so...
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY); // Index 1 since the second buffer is the RED_INTEGER buffer
			m_HoveredEntity = pixelData == -1 ? Entity::nullEntity : Entity{ (entt::entity)pixelData, m_ActiveScene.raw()};
		}
		
		m_Framebuffer->UnBind();

		if (Input::IsMouseButtonPressed(MouseButton::ButtonRight) && !m_StartedRightClickInViewport && m_ViewportFocused && m_ViewportHovered)
			m_StartedRightClickInViewport = true;

		if (!Input::IsMouseButtonPressed(MouseButton::ButtonRight))
			m_StartedRightClickInViewport = false;
	}

	void EditorLayer::OnTick()
	{
		const std::unordered_map<const char*, float>& mp = Application::GetApp().GetPerformanceProfiler()->GetPerFrameData();

		m_SortedTimerValues.clear();
		m_SortedTimerValues.reserve(mp.size());
		for (auto& [name, val] : mp)
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
		dispatcher.dispatch<KeyPressedEvent>(AR_SET_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.dispatch<MouseButtonPressedEvent>(AR_SET_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (GImGui->ActiveId == 0)
		{
			bool control = Input::IsKeyPressed(AR_KEY_LEFT_CONTROL) || Input::IsKeyPressed(AR_KEY_RIGHT_CONTROL);
			bool shift = Input::IsKeyPressed(AR_KEY_LEFT_SHIFT) || Input::IsKeyPressed(AR_KEY_RIGHT_SHIFT);
			bool alt = Input::IsKeyPressed(AR_KEY_LEFT_ALT) || Input::IsKeyPressed(AR_KEY_RIGHT_ALT);

			bool isSomethingSelected = m_SelectionContext ? true : false;

			switch (e.GetKeyCode())
			{
				// New Scene
			    case AR_KEY_N:
			    {
			    	if (control)
			    		NewScene();
			    
			    	break;
			    }
			    
				// Open Scene
			    case AR_KEY_O:
			    {
			    	if (control)
			    		OpenScene();
			    
			    	break;
			    }
			    
				// Save Scene
			    case AR_KEY_S:
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
			    case AR_KEY_D:
			    {
			    	// TODO: Needs rework...
			    	if (control && isSomethingSelected)
			    	{
			    		m_SelectionContext = m_ActiveScene->CopyEntity(m_SelectionContext);
			    	}
			    
			    	break;
			    }
			    
				// Delete entity
			    case AR_KEY_DELETE:
			    {
			    	if (isSomethingSelected)
			    	{
			    		m_ActiveScene->DestroyEntity(m_SelectionContext);
			    		m_SelectionContext = {};
			    	}
			    
			    	break;
			    }
			    
			    // Gizmos
			    case AR_KEY_Q:
			    {
			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = -1;
			    
			    	break;
			    }
			    
			    case AR_KEY_W:
			    {
			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			    
			    	break;
			    }
			    
			    case AR_KEY_E:
			    {
			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			    
			    	break;
			    }
			    
			    case AR_KEY_R:
			    {
			    	if (!ImGuizmo::IsUsing())
			    		m_GizmoType = ImGuizmo::OPERATION::SCALE;
			    
			    	break;
			    }

				// Reset Editor Camera
				case AR_KEY_C:
				{
					if (alt)
					{
						m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 10000.0f);
					}
				}

				// Reset focal point
				case AR_KEY_F:
				{
					if (alt)
					{
						m_EditorCamera.Focus({ 0.0f, 0.0f, 0.0f });
					}
				}
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		bool isImGuizmoOver = ImGuizmo::IsOver();
		bool isAltPressed = Input::IsKeyPressed(AR_KEY_LEFT_ALT);
		bool inSceneHierarchyTableRect = ImGuiUtils::IsMouseInRectRegion(m_SceneHierarchyTableRect, false);
		bool inViewportRect = ImGuiUtils::IsMouseInRectRegion(m_ViewportRect, false);
		bool isAnyImGuiItemHovered = ImGui::IsAnyItemHovered();
		bool leftCLick = e.GetButtonCode() == AR_MOUSE_BUTTON_LEFT;

		// I want to control the selection ONLY IN THE SCENE HIERARCHY AND IN THE VIEWPORT!
		if (leftCLick && !isImGuizmoOver && !isAltPressed && !isAnyImGuiItemHovered && (inSceneHierarchyTableRect || inViewportRect))
		{
			if (m_HoveredEntity != Entity::nullEntity && inViewportRect)
				m_SelectionContext = m_HoveredEntity;
			else
				m_SelectionContext = {};

			return true;
		}

		return false;
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
		// Assuming all components have a tag component
		const std::string& tag = entity.GetComponent<TagComponent>().Tag;

		if (!ImGuiUtils::IsMatchingSearch(tag, searchedString, false, true))
			return;

		std::string strID = fmt::format("{}{}", tag, entity.GetUUID());

		const float edgeOffset = 4.0f;
		const float rowHeight = 21.0f;

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

		ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_AllowItemOverlap
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
			fillRowWithColour(ImColor(236, 158, 36, 150));
		else if (isHovered)
			fillRowWithColour(IM_COL32(236, 158, 36, 200));

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
		ImGui::PopStyleColor(3);

		bool inSceneHierarchyTable = ImGuiUtils::IsMouseInRectRegion(m_SceneHierarchyTableRect, false);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& ImGuiUtils::IsMouseInRectRegion({ rowAreaMin, rowAreaMax }, false)
			&& inSceneHierarchyTable)
		{
			m_SelectionContext = entity;
		}

		bool mouseClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::TableGetRowIndex())
			|| (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && Input::IsKeyPressed(AR_KEY_LEFT_SHIFT) 
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
		ImGui::Text("Entity");

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
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene Hierarchy", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);

		static bool enableSorting = false;
		const float edgeOffset = 4.0f;

		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::TextColored(ImVec4{ 0.925f, 0.619f, 0.141f, 0.888f }, "Scene:");

		ImGui::SameLine();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f - 30.0f);

		std::string& sceneName = m_EditorScene->GetName();
		// There should not be a scene name that is more than 128 characters long LOL
		char buffer[128];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), sceneName.c_str());
		if(ImGui::InputTextWithHint(ImGuiUtils::GenerateID(), "Enter scene name...", buffer, sizeof(buffer)))
		{
			sceneName = std::string(buffer);
		}

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
			ImGui::Text("Show Properties: ");

			ImGui::SameLine();

			if (ImGui::Checkbox("##ShowProperties", &m_ShowPropertiesPanel))
				ImGui::CloseCurrentPopup();

			ImGui::Text("Enable Sorting: ");

			ImGui::SameLine(0.0f, 20.0f);

			if (ImGui::Checkbox("##EnableSorting", &enableSorting))
				ImGui::CloseCurrentPopup();

			ImGui::Separator();

			ImGui::TextColored(ImVec4{ 0.3f, 0.9f, 0.3f, 1.0f }, "To deselect an entity:\nShift + LeftClick");
			//ImGui::Text("To deselect an entity:\nShift + LeftClick");

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

			ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX
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
			ImGui::Begin("Properties");

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
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			T& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.0f, 4.0f });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f; // From ImGui source
			ImGui::Separator();

			// TODO: Add component frame icons
			//bool open = ImGui::TreeNodeWithIcon(EditorResources::TransformCompIcon, (void*)typeid(T).hash_code(), treeNodeFlags, ImColor(255, 255, 255, 255), name.c_str());
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				void* textureID = (void*)(uint64_t)EditorResources::CloseIcon->GetTextureID();
				if (ImGui::ImageButton(textureID, ImVec2{ lineHeight - 9.0f, lineHeight - 6.0f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }))
				//if (ImGui::Button("X", ImVec2{ lineHeight, lineHeight }))
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
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
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

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		layer->m_FontsLibrary.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
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
		layer->m_FontsLibrary.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
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
		layer->m_FontsLibrary.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
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

	template<typename Component>
	static void DrawPopUpMenuItems(std::string_view compName, Entity entity, Entity& selectionContext)
	{
		if (typeid(Component).hash_code() != typeid(ModelComponent).hash_code())
		{
			if (!entity.HasComponent<Component>())
			{
				if (ImGui::MenuItem(compName.data()))
				{
					selectionContext.AddComponent<Component>();
					ImGui::CloseCurrentPopup();
				}
			}
		}
		else
		{
			if (!entity.HasComponent<Component>())
			{
				if (ImGui::MenuItem(compName.data()))
				{
					std::filesystem::path path = Utils::WindowsFileDialogs::OpenFileDialog("Model file");

					if (path != "")
					{
						selectionContext.AddComponent<ModelComponent>(path.string());
					}

					ImGui::CloseCurrentPopup();
				}
			}
		}
	}

	void EditorLayer::DrawComponents(Entity entity)
	{
		std::string& tag = entity.GetComponent<TagComponent>().Tag;

		// Size of buffer is 128 because there should not be an entity called more than 128 letters like BRUH
		char buffer[128];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), tag.c_str());
		if (ImGui::InputTextWithHint("##Tag", "Change entity name...", buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}

		ImGuiUtils::DrawItemActivityOutline(3.0f, true, Theme::Accent);

		if (ImGui::IsItemHovered())
			ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "ID: %#010x", entity.GetUUID());

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponentsPopUp"); // AddComponentPopUp here acts as an ID for imgui to be used in the next step

		ImGui::PopItemWidth();

		if (ImGui::BeginPopup("AddComponentsPopUp"))
		{
			DrawPopUpMenuItems<CameraComponent>("Camera", entity, m_SelectionContext);
			DrawPopUpMenuItems<SpriteRendererComponent>("Sprite Renderer", entity, m_SelectionContext);
			DrawPopUpMenuItems<ModelComponent>("Model Component", entity, m_SelectionContext);

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
			auto& camera = component.Camera;

			ImGui::Columns(2);

			ImGui::SetColumnWidth(0, 100.0f);
			ImGui::Text("Primary");

			ImGui::NextColumn();

			ImGui::Checkbox("##Primary", &component.Primary);

			static const char* projectionTypeString[] = { "Perspective", "Orthographic" };
			static const char* currentProjectionTypeString = projectionTypeString[(int)camera.GetProjectionType()];

			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::Text("Projection");
			ImGui::NextColumn();

			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##Projection", currentProjectionTypeString))
			{
				for (int type = 0; type < 2; type++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeString[type];
					if (ImGui::Selectable(projectionTypeString[type], &isSelected))
					{
						currentProjectionTypeString = projectionTypeString[type];
						camera.SetProjectionType((SceneCamera::ProjectionType)type);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			ImGui::Separator();

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				ImGui::Text("FOV");
				if (ImGui::IsItemHovered())
					ImGuiUtils::ToolTip("Filed Of View");

				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("##Vertical FOV", &verticalFOV, 0.1f))
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));
				ImGui::PopItemWidth();

				ImGui::NextColumn();
				ImGui::Separator();

				ImGui::Text("Near");
				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float persNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("##Near", &persNear, 0.1f))
					camera.SetPerspectiveNearClip(persNear);
				ImGui::PopItemWidth();

				ImGui::NextColumn();
				ImGui::Separator();

				ImGui::Text("Far");
				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float persFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("##Far", &persFar, 0.1f))
					camera.SetPerspectiveFarClip(persFar);
				ImGui::PopItemWidth();

				ImGui::Columns(1);
			}

			else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				ImGui::Text("Size");
				if (ImGui::IsItemHovered())
					ImGuiUtils::ToolTip("Controls the top, bottom, left and right\nboundaries of the camera.");

				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("##Size", &orthoSize, 0.1f))
					camera.SetOrthographicSize(orthoSize);
				ImGui::PopItemWidth();

				ImGui::NextColumn();
				ImGui::Separator();

				ImGui::Text("Near");
				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("##Near", &orthoNear, 0.1f))
					camera.SetOrthographicNearClip(orthoNear);
				ImGui::PopItemWidth();

				ImGui::NextColumn();
				ImGui::Separator();

				ImGui::Text("Far");
				ImGui::NextColumn();

				ImGui::PushItemWidth(-1);
				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("##Far", &orthoFar, 0.1f))
					camera.SetOrthographicFarClip(orthoFar);
				ImGui::PopItemWidth();

				ImGui::Columns(1);
			}
		},
		[](CameraComponent& component) // Reset Function
		{
			auto& camera = component.Camera;
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				camera.SetPerspectiveVerticalFOV(glm::radians(45.0f));
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

		DrawComponent<ModelComponent>("Model", entity, [](ModelComponent& component)
		{
			std::string path = component.model.directory;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), path.c_str());
			ImGui::InputTextWithHint("##Filepath", "Filepath...", buffer, sizeof(buffer));
		},
		[](ModelComponent& component) // Reset Function
		{
			AR_DEBUG("WASSUP NOTHING TO RESET");
		});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			ImGui::Columns(2);

			ImGui::SetColumnWidth(0, 100.0f);
			ImGui::Text("Color");
			ImGui::NextColumn();

			ImGui::PushItemWidth(-1);
			ImGui::ColorEdit4("##Color", glm::value_ptr(component.Color));
			ImGui::PopItemWidth();

			ImGui::Separator();
		},
		[](SpriteRendererComponent& component) // Reset Function
		{
			glm::vec4& color = component.Color;
			color = glm::vec4(1.0f);
		});
	}

#pragma endregion

	// TODO: NEED TO CHANGE SOME STUFF HERE FOR LATER
#pragma region FileDialogs/Scene Helpers

	void EditorLayer::NewScene()
	{
		m_ActiveScene->Clear();
		m_ActiveScene = Scene::Create("New Scene"); // Creating new scene
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		SetContextForSceneHeirarchyPanel(m_ActiveScene);

		Entity plane = m_ActiveScene->CreateEntity();
		m_GroundEntity = plane;
		TagComponent& name = m_GroundEntity.GetComponent<TagComponent>();
		name.Tag = "Plane";
		TransformComponent& tc = m_GroundEntity.GetComponent<TransformComponent>();
		tc.Scale = glm::vec3{ 200.0f, 0.5f, 200.0f };
		SpriteRendererComponent& sp = m_GroundEntity.AddComponent<SpriteRendererComponent>();
		sp.Color = { 0.6f, 0.6f, 0.6f, 1.0f };

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
			AR_WARN("Could not load '{0}' - not an Aurora scene file!", path.filename().string());
			
			return;
		}

		Ref<Scene> newScene = Scene::Create();
		SceneSerializer serializer(newScene);

		if (serializer.DeSerializeFromText(path.string()))
		{
			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
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

#pragma endregion

#pragma region RendererPanels

	void EditorLayer::ShowRendererVendorInfoUI()
	{
		ImGui::Begin("Renderer Vendor", &m_ShowRendererVendorInfo, ImGuiWindowFlags_AlwaysAutoResize);

		// TODO: Change this so that the renderprops are retreived from the Rendere class which then calls renderProps::Get...()...
		ImGui::Text("Vendor: %s", RendererProperties::GetRendererProperties()->Vendor.c_str());
		ImGui::Text("Renderer: %s", RendererProperties::GetRendererProperties()->Renderer.c_str());
		ImGui::Text("OpenGL Version: %s", RendererProperties::GetRendererProperties()->Version.c_str());
		ImGui::Text("GLSL Version: %s", RendererProperties::GetRendererProperties()->GLSLVersion.c_str());
		ImGui::Text("Texture Slots Available: %d", RendererProperties::GetRendererProperties()->MaxTextureSlots);
		ImGui::Text("Max Samples: %d", RendererProperties::GetRendererProperties()->MaxSamples);
		ImGui::Text("Max Anisotropy: %.f", RendererProperties::GetRendererProperties()->MaxAnisotropy);

		ImGui::End();
	}

	void EditorLayer::ShowRendererStatsUI()
	{
		ImGui::Begin("Renderer Stats");

		float peak = std::max(m_Peak, ImGui::GetIO().Framerate);
		m_Peak = peak;

		//std::string name = "None";
		//if (m_HoveredEntity && ImGuiUtils::IsMouseInRectRegion(m_ViewportRect, false))
		//	name = m_HoveredEntity.GetComponent<TagComponent>().Tag;

		//ImGui::Text("Hovered Entity: %s", name.c_str());
		ImGui::Text("Draw Calls: %d", Renderer3D::GetStats().DrawCalls);
		ImGui::Text("Quad Count: %d", Renderer3D::GetStats().QuadCount);
		ImGui::Text("Vertex Count: %d", Renderer3D::GetStats().GetTotalVertexCount());
		ImGui::Text("Index Count: %d", Renderer3D::GetStats().GetTotalIndexCount());
		ImGui::Text("Vertex Buffer Usage: %.3f Megabytes", Renderer3D::GetStats().GetTotalVertexBufferMemory() / (1024.0f * 1024.0f));

		static bool wireFrame = false;
		static bool vertices = false;
		static bool VSyncState = true;
		if (ImGui::Checkbox("WireFrame", &wireFrame))
		{
			RenderCommand::SetRenderFlag(RenderFlags::WireFrame);
			vertices = false;
		}

		if (ImGui::Checkbox("Vertices", &vertices))
		{
			RenderCommand::SetRenderFlag(RenderFlags::Vertices);
			wireFrame = false;
		}
		if (!wireFrame && !vertices)
			RenderCommand::SetRenderFlag(RenderFlags::Fill);

		if (ImGui::Checkbox("V-Sync", &VSyncState))
		{
			Application::GetApp().GetWindow().SetVSync(VSyncState);
		}

		ImGui::End();
	}

	void EditorLayer::ShowShadersPanel()
	{
		ImGui::Begin("Shaders", &m_ShowShadersPanel);

		const float edgeOffset = 4.0f;
		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f);

		static std::string searchString;
		ImGuiUtils::SearchBox(searchString);

		ImGui::Spacing();
		ImGui::Spacing();

		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		for (Ref<Shader> shader : Shader::s_AllShaders)
		{
			const std::string& shaderName = shader->GetName();

			if (!ImGuiUtils::IsMatchingSearch(shaderName, searchString))
				continue;

			const float framePaddingX = 5.0f;
			const float framePaddingY = 5.0f; // affects height of the header

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });
			const bool opened = ImGui::TreeNodeEx(shaderName.c_str(), treeNodeFlags);
			ImGui::PopStyleVar(2);

			ImGuiUtils::ShiftCursorY(-4.0f);

			if(opened)
			{
				ImGui::Text("Path: %s", shader->GetFilePath().c_str());
				if (ImGui::Button("Reload"))
					shader->Reload(true);

				ImGui::TreePop();
			}
		}

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

		// TODO: This is super temporary since im just working on materials for now...
		ImGui::ColorEdit3("Material Tint", glm::value_ptr(s_MaterialAlbedoColor));

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;
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
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;

		ImGui::Begin("Editor Style", &m_ShowEditingPanel);

		if (ImGui::TreeNodeEx("Font", flags))
		{
			ShowFontPickerUI();

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Editor Style", flags))
		{
			ImGui::ShowStyleEditor();

			ImGui::TreePop();
		}

		ImGui::End();
	}

#pragma endregion

#pragma region FontPickerUI

	void EditorLayer::ShowFontPickerUI()
	{
		ImGuiLayer* layer = Application::GetApp().GetImGuiLayer();

		std::vector<std::tuple<std::string, std::string, FontIdentifier>> sortedFontNames;
		sortedFontNames.reserve(layer->m_FontsLibrary.GetFontNamesAndIdentifier().size());

		const char* idenType;
		std::string displayName;

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);

		ImGui::Text("Editor Font:");

		ImGui::NextColumn();

		ImGui::PushItemWidth(-1);
		if (ImGui::BeginCombo("##FontPicker", m_SelectedFontName.c_str()))
		{
			for (const auto&[pair, second] : layer->m_FontsLibrary.GetFontNamesAndIdentifier())
			{
				switch (pair.second)
				{
				    case FontIdentifier::Bold:       idenType = ", Bold"; break;
				    case FontIdentifier::Italic:     idenType = ", Italic"; break;
				    case FontIdentifier::Regular:    idenType = ", Regular"; break;
				    case FontIdentifier::Medium:     idenType = ", Medium"; break;
				    case FontIdentifier::Light:      idenType = ", Light"; break;
				}

				displayName = pair.first;
				displayName.append(idenType);
				sortedFontNames.emplace_back(displayName, pair.first, pair.second);
			}

			std::sort(sortedFontNames.begin(), sortedFontNames.end());
			for (const auto& [displayName, fontName, type] : sortedFontNames)
			{
				layer->m_FontsLibrary.SetTemporaryFont(fontName, type);
				if (ImGui::Selectable(displayName.c_str()))
				{
					layer->m_FontsLibrary.SetDefaultFont(fontName, type);
					m_SelectedFontName = displayName;
				}
				layer->m_FontsLibrary.PopTemporaryFont();
			}

			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		ImGui::Columns(1);
	}

#pragma endregion

#pragma region HelpPanels/UI

	void EditorLayer::ShowEditorCameraHelpUI()
	{
		ImGui::Begin("Editor Camera Controls:", &m_ShowEditorCameraHelpUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("These are the editor camera's controls:");
		ImGui::Text("RightClick + Use W/A/S/D/Q/E to control the camera");
		ImGui::Text("RightClick + MouseScroll to increase the movement speed of fps Camera");
		ImGui::Text("Press LeftAlt + MouseRightClick and move to Rotate.");
		ImGui::Text("Press LeftAlt + MouseLeftClick and move to Pan.");
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
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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

				ImGui::Separator();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				ImGui::Separator();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				ImGui::Separator();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Scene Hierarchy", NULL, m_ShowSceneHierarchyPanel))
					m_ShowSceneHierarchyPanel = !m_ShowSceneHierarchyPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Shaders", NULL, m_ShowShadersPanel))
					m_ShowShadersPanel = !m_ShowShadersPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Performance", NULL, m_ShowPerformance)) 
					m_ShowPerformance = !m_ShowPerformance;

				ImGui::Separator();

				if (ImGui::MenuItem("Renderer Stats", NULL, m_ShowRenderStatsUI))
					m_ShowRenderStatsUI = !m_ShowRenderStatsUI;

				ImGui::Separator();

				if (ImGui::MenuItem("Renderer Info", NULL, m_ShowRendererVendorInfo)) 
					m_ShowRendererVendorInfo = !m_ShowRendererVendorInfo;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Settings...", NULL, m_ShowSettingsUI))
					m_ShowSettingsUI = !m_ShowSettingsUI;

				ImGui::Separator();

				if (ImGui::MenuItem("Exit...", NULL, m_ShowCloseModal))
					m_ShowCloseModal = !m_ShowCloseModal;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Editor Style", NULL, m_ShowEditingPanel))
					m_ShowEditingPanel = !m_ShowEditingPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("ImGui StackTool", NULL, m_ShowDearImGuiStackToolWindow))
					m_ShowDearImGuiStackToolWindow = !m_ShowDearImGuiStackToolWindow;

				ImGui::Separator();

#ifdef AURORA_DEBUG
				if (ImGui::MenuItem("ImGui DebugLog", NULL, m_ShowDearImGuiDebugLogWindow))
					m_ShowDearImGuiDebugLogWindow = !m_ShowDearImGuiDebugLogWindow;

				ImGui::Separator();
#endif

				if (ImGui::MenuItem("ImGui Metrics", NULL, m_ShowDearImGuiMetricsWindow))
					m_ShowDearImGuiMetricsWindow = !m_ShowDearImGuiMetricsWindow;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Editor Camera", NULL, m_ShowEditorCameraHelpUI))
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
		glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

		// Runtime Camera
		// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
		// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		// const glm::mat4& cameraProjection = camera.GetProjection();
		// glm::mat4 cameraView2 = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

		// Entity transform
		auto& tc = m_SelectionContext.GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		// Snapping
		bool snap = Input::IsKeyPressed(AR_KEY_LEFT_CONTROL);

		float snapValue = GetSnapValue();

		float snapValues[3] = { snapValue, snapValue, snapValue };

		if(ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
			nullptr, snap ? snapValues : nullptr) && !Input::IsKeyPressed(AR_KEY_LEFT_ALT))
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
		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = viewPortPanelSize;

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_EditorCamera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		void* textureID = (void*)(uint64_t)m_Framebuffer->GetColorAttachmentID();
		ImGui::Image(textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		// This displays the textures used in the editor if i ever want to display a texture pretty quick just change the textID
		//ImGui::Image((ImTextureID)EditorResources::ResetIcon->GetTextureID(), { m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		m_AllowViewportCameraEvents = (ImGuiUtils::IsMouseInRectRegion(m_ViewportRect, true) && m_ViewportFocused) || m_StartedRightClickInViewport;

		ImGuiUtils::SetInputEnabled(!m_ViewportFocused || !m_ViewportHovered);

		// Gizmos...
		if (m_SelectionContext != Entity::nullEntity && m_GizmoType != -1)
		{
			float windowWidth = m_ViewportRect.Max.x - m_ViewportRect.Min.x;
			float windowHeight = m_ViewportRect.Max.y - m_ViewportRect.Min.y;

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportRect.Min.x, m_ViewportRect.Min.y, windowWidth, windowHeight);

			ManipulateGizmos();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	// TODO: Probably needs to be changed lol hard work for nothing xD
	template<typename UIFunction>
	static void DrawSettingsFeatureCheckbox(const std::string& name, const std::string& description, bool* controller, FeatureControl feature, UIFunction func)
	{
		ImGui::Text((name + ":").c_str());
		ImGui::SameLine();
		ImGuiUtils::ShowHelpMarker(description.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 200.0f);
		
		ImGui::Text("Enabled: ");
		ImGui::NextColumn();
		ImGui::Checkbox(("##" + name).c_str(), controller);

		ImGui::Columns(1);

		if (!(*controller))
			RenderCommand::Disable(feature);
		else
			RenderCommand::Enable(feature);

		func();
	}

	// TODO: Probably needs to be changed lol hard work for nothing xD
	void EditorLayer::ShowSettingsUI()
	{
		// TODO: Make it so when someone changes the global settings of the editor they are saved in an auroraeditor.ini file and loaded when reopened
		// Control settings such as enable back face culling, depth testing, blending, blend function...
		// TODO: Add explanation in the HelpMarker for each setting in each combo!
		static bool enableCulling = true;
		static bool enableBlending = true;
		static bool enableDepthTesting = true;

		static std::string cullOption = "Back";
		static std::string blendOption = "One Minus Source Alpha";
		static std::string depthOption = "Less";
		static std::string blendEquation = "Add";
		static float TickDelta = 1.0f;

		ImGui::Begin("Settings", &m_ShowSettingsUI);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 200.0f);

		ImGui::Text("App Tick Delta: ");

		ImGui::NextColumn();

		ImGui::SetNextItemWidth(-1);
		if (ImGui::SliderFloat("##AppTickingDelta", &TickDelta, 0.0f, 5.0f))
			Application::GetApp().SetTickDeltaTime(TickDelta);

		ImGui::Columns(1);

		ImGui::Separator();

		std::string blendDesc = "Blending is the technique that allows and implements the \"Transparency\" within objects.";
		DrawSettingsFeatureCheckbox("Blending", blendDesc, &enableBlending, FeatureControl::Blending, []()
		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 200.0f);

			ImGui::Text("Blending Function");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##BlendingFunction", blendOption.c_str()))
			{
				if (ImGui::Selectable("Zero"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::Zero);
					blendOption = "Zero";
				}

				else if (ImGui::Selectable("One"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::One);
					blendOption = "One";
				}

				else if (ImGui::Selectable("Source Color"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::SrcColor);
					blendOption = "Source Color";
				}

				else if (ImGui::Selectable("Source Alpha"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::SrcAlpha);
					blendOption = "Source Alpha";
				}

				else if (ImGui::Selectable("Destination Color"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::DstColor);
					blendOption = "Destination Color";
				}

				else if (ImGui::Selectable("One Minus Source Color"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::OneMinusSrcColor);
					blendOption = "One Minus Source Color";
				}

				else if (ImGui::Selectable("One Minus Source Alpha"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::OneMinusSrcAlpha);
					blendOption = "One Minus Source Alpha";
				}

				else if (ImGui::Selectable("One Minus Destination Color"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Blending, OpenGLFunction::OneMinusDstColor);
					blendOption = "One Minus Destination Color";
				}

				ImGui::Columns(1);
				ImGui::EndCombo();
			}
		});

		// Blend equation
		ImGui::NextColumn();
		ImGui::Text("Blending Equation");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##BlendingEquation", blendEquation.c_str())) // TODO: Fix the naming to display in the combo label
		{
			if (ImGui::Selectable("Add"))
			{
				RenderCommand::SetBlendFunctionEquation(OpenGLEquation::Add);
				blendEquation = "Add";
			}

			else if (ImGui::Selectable("Subtract"))
			{
				RenderCommand::SetBlendFunctionEquation(OpenGLEquation::Subtract);
				blendEquation = "Subtract";
			}

			else if (ImGui::Selectable("Reverse Subtract"))
			{
				RenderCommand::SetBlendFunctionEquation(OpenGLEquation::ReverseSubtract);
				blendEquation = "Reverse Subtract";
			}

			else if (ImGui::Selectable("Minimum"))
			{
				RenderCommand::SetBlendFunctionEquation(OpenGLEquation::Minimum);
				blendEquation = "Minimum";
			}

			else if (ImGui::Selectable("Maximum"))
			{
				RenderCommand::SetBlendFunctionEquation(OpenGLEquation::Maximum);
				blendEquation = "Maximum";
			}

			ImGui::Columns(1);
			ImGui::EndCombo();
		}

		ImGui::Separator();

		ImGui::Columns(1);
		std::string cullingDesc = "Culling is used to specify to the renderer what face is not to be processed thus reducing processing time.";
		DrawSettingsFeatureCheckbox("Culling", cullingDesc, &enableCulling, FeatureControl::Culling, []()
		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 200.0f);

			ImGui::Text("Culling Function");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##CullingFunction", cullOption.c_str())) // TODO: Fix the naming to display in the combo label
			{
				if (ImGui::Selectable("Back"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Culling, OpenGLFunction::Back);
					cullOption = "Back";
				}

				else if (ImGui::Selectable("Front"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Culling, OpenGLFunction::Front);
					cullOption = "Front";
				}

				else if (ImGui::Selectable("Front and Back"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::Culling, OpenGLFunction::FrontAndBack);
					cullOption = "FronAndBack";
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);

		});

		ImGui::Separator();

		std::string depthDesc = "Depth testing is what allows for 3D objects to appear on the screen via a depth buffer.";
		DrawSettingsFeatureCheckbox("DepthTesting", depthDesc, &enableDepthTesting, FeatureControl::DepthTesting, []()
		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 200.0f);

			ImGui::Text("Depth Function");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##DepthFunction", depthOption.c_str()))
			{
				if (ImGui::Selectable("Never"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Never);
					depthOption = "Never";
				}

				else if (ImGui::Selectable("Equal"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Equal);
					depthOption = "Equal";
				}

				else if (ImGui::Selectable("Not Equal"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::NotEqual);
					depthOption = "Not Equal";
				}

				else if (ImGui::Selectable("Less"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Less);
					depthOption = "Less";
				}

				if (ImGui::Selectable("Less Or Equal"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::LessOrEqual);
					depthOption = "Less Or Equal";
				}

				else if (ImGui::Selectable("Greater"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Greater);
					depthOption = "Greater";
				}

				else if (ImGui::Selectable("Greater Or Equal"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::GreaterOrEqual);
					depthOption = "Greater Or Equal";
				}

				else if (ImGui::Selectable("Always"))
				{
					RenderCommand::SetFeatureControlFunction(FeatureControl::DepthTesting, OpenGLFunction::Always);
					depthOption = "Always";
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);
		});

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

		// TODO: THIS IS FOR REFERENCE ONLY!
		//ImGui::ShowDemoWindow();

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

		if (m_ShowPerformance)
			ShowPerformanceUI();

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