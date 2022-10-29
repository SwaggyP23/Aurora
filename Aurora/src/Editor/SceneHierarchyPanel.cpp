#include "Aurorapch.h"
#include "SceneHierarchyPanel.h"

#include "AssetManager/AssetManager.h"

#include "Core/Application.h"
#include "Core/Input/Input.h"

#include "ImGui/FontAwesome.h"
#include "ImGui/ImGuiLayer.h"
#include "ImGui/ImGuiUtils.h"

#include "Utils/UtilFunctions.h"

namespace Aurora {

	static bool s_ActivateSearchWidget = false;
	static SelectionContext s_ActiveSelectionContext = SelectionContext::Scene;

	namespace Utils {

		template<typename Component, bool TAddSeparator = false>
		static void DrawPopUpMenuEntry(SceneHierarchyPanel* SHP, std::string_view compName)
		{
			bool canAdd = false;
			
			for (const UUID& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
			{
				Entity entity = SHP->GetSceneContext()->GetEntityWithUUID(entityID);
				if (!entity.HasComponent<Component>())
				{
					canAdd = true;
					break;
				}
			}

			if (!canAdd)
				return;

			if (ImGui::MenuItem(compName.data()))
			{
				for (const UUID& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
				{
					Entity entity = SHP->GetSceneContext()->GetEntityWithUUID(entityID);

					entity.AddComponent<Component>();
				}

				ImGui::CloseCurrentPopup();
			}

			if constexpr (TAddSeparator)
			{
				ImGui::Separator();
			}
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
					if (ImGui::Button(AR_ICON_TRASH, ImVec2{ lineHeight, lineHeight }))
					{
						ImGui::OpenPopup("ComponentSettings"); // This is also just an id and not a tag that will be rendered
					}

					ImGui::SameLine(contentRegionAvail.x - 1.5f * lineHeight);

					if (ImGui::Button(AR_ICON_REFRESH, ImVec2{ lineHeight, lineHeight }))
					{
						resetFunction(component);
					}

					if (ImGui::IsItemHovered())
						ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "Reset");
				}
				else
				{
					ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);

					if (ImGui::Button(AR_ICON_REFRESH, ImVec2{ lineHeight, lineHeight }))
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

	}

	Ref<SceneHierarchyPanel> SceneHierarchyPanel::Create()
	{
		return CreateRef<SceneHierarchyPanel>();
	}

	Ref<SceneHierarchyPanel> SceneHierarchyPanel::Create(const Ref<Scene>& scene, SelectionContext context)
	{
		return CreateRef<SceneHierarchyPanel>(scene, context);
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene, SelectionContext context)
		: m_Context(scene)
	{
		s_ActiveSelectionContext = context;

		if (m_Context)
			m_Context->SetEntityDestroyedCallback([this](Entity entity) { OnExternalEntityDestroyed(entity); });
	}

	void SceneHierarchyPanel::SetSceneContext(const Ref<Scene>& scene)
	{
		m_Context = scene;

		if (m_Context)
			m_Context->SetEntityDestroyedCallback([this](Entity entity) { OnExternalEntityDestroyed(entity); });
	}

	SelectionContext SceneHierarchyPanel::GetActiveSelectionContext()
	{
		return s_ActiveSelectionContext;
	}

	void SceneHierarchyPanel::OnImGuiRender(bool& isOpen)
	{
		ImGuiFontsLibrary& fontsLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene Hierarchy", &isOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);

		m_IsPanelFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		static bool enableSorting = false;
		constexpr float edgeOffset = 4.0f;

		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		fontsLib.PushTemporaryFont("RobotoBold");
		ImGui::TextColored(ImVec4{ 0.925f, 0.619f, 0.141f, 0.888f }, "Scene:");

		ImGui::SameLine();

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f - 30.0f);

		std::string& sceneName = m_Context->GetName();
		// There should not be a scene name that is more than 128 characters long LOL
		char buffer[128];
		strcpy_s(buffer, sizeof(buffer), sceneName.c_str());
		ImGuiUtils::ShiftCursorY(-2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		if (ImGui::InputTextWithHint(ImGuiUtils::GenerateID(), "Enter scene name...", buffer, sizeof(buffer)))
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

		if (s_ActivateSearchWidget)
		{
			ImGui::SetKeyboardFocusHere();
			s_ActivateSearchWidget = false;
		}

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

				if (enableSorting)
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

		// TODO: This is for un parenting entities
		//if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
		//{
		//	const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("scene_entity_hierarchy_pay", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

		//	if (payLoad)
		//	{
		//	}

		//	ImGui::EndDragDropTarget();
		//}

		if (m_ShowPropertiesPanel)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8, 8 });
			ImGui::Begin("Properties", (bool*)0, ImGuiWindowFlags_NoMove);
			m_IsHierarchyOrPropertiesFocused = m_IsPanelFocused || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

			DrawComponents(SelectionManager::GetSelections(s_ActiveSelectionContext));

			ImGui::End();
			ImGui::PopStyleVar();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneHierarchyPanel::OnEvent(Event& e)
	{
		if (!m_IsPanelFocused)
			return;

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressedEvent(e); });
	}

	bool SceneHierarchyPanel::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		if (Input::IsKeyDown(KeyCode::LeftControl))
		{
			switch (e.GetKeyCode())
			{
			    case KeyCode::F:
			    {
			    	s_ActivateSearchWidget = true;
			    	break;
			    }
			}
		}

		return false;
	}

	// TODO: Expand
	void SceneHierarchyPanel::DrawEntityCreatePopupMenu(Entity entity)
	{
		if (!ImGui::BeginMenu("Create"))
			return;

		Entity newEntity;

		if (ImGui::MenuItem("Empty Entity"))
		{
			newEntity = m_Context->CreateEntity("Empty Entity");
		}

		if (ImGui::MenuItem("Camera"))
		{
			newEntity = m_Context->CreateEntity("Camera");
			newEntity.AddComponent<CameraComponent>();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("DirectionalLight"))
		{
			newEntity = m_Context->CreateEntity("DirectionalLight");
			newEntity.AddComponent<DirectionalLightComponent>();
			glm::vec3& rotation = newEntity.GetComponent<TransformComponent>().Rotation;
			rotation = glm::radians(glm::vec3(80.0f, 10.0f, 0.0f));
		}

		if (ImGui::MenuItem("SkyLight"))
		{
			newEntity = m_Context->CreateEntity("SkyLight");
			newEntity.AddComponent<SkyLightComponent>();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Text"))
		{
			newEntity = m_Context->CreateEntity("Text");
			TextComponent& textComp = newEntity.AddComponent<TextComponent>();
			textComp.FontHandle = Font::GetDefaultFont()->Handle;
		}

		if (newEntity)
		{
			SelectionManager::DeselectAll();
			SelectionManager::Select(s_ActiveSelectionContext, newEntity.GetUUID());
		}

		ImGui::EndMenu();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, const std::string& searchedString)
	{		
		constexpr ImGuiButtonFlags buttonFlags = ImGuiButtonFlags_AllowItemOverlap
			| ImGuiButtonFlags_PressedOnClickRelease
			| ImGuiButtonFlags_MouseButtonLeft
			| ImGuiButtonFlags_MouseButtonRight;

		// All entities have a tag component by default
		const std::string& tag = entity.GetComponent<TagComponent>().Tag;

		if (!ImGuiUtils::IsMatchingSearch(tag, searchedString, false, true))
			return;

		std::string strID = fmt::format("{}{}", tag, entity.GetUUID());

		constexpr float edgeOffset = 4.0f;
		constexpr float rowHeight = 21.0f;

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

		bool isHovered = false;
		bool isHeld = false;
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

		const bool isSelected = SelectionManager::IsSelected(s_ActiveSelectionContext, entity.GetUUID());
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
			: ImVec2{ style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y) };

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
			SelectionManager::DeselectAll();
			SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());
		}

		const bool mouseClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::TableGetRowIndex())
			|| (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && Input::IsKeyDown(KeyCode::LeftShift)
				&& inSceneHierarchyTable);

		if (mouseClicked)
			SelectionManager::DeselectAll();

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
		ImGui::PopStyleColor(4);

		// TODO: Add other identifiers when there actually are other identifiers such as prefabs when these are a thing
		ImGui::TableNextColumn();
		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextBrighter);
		else if (isHovered)
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::Background);
		ImGui::Text("Entity");
		if (isHovered || isSelected)
			ImGui::PopStyleColor();

		// Mouse Click events also for this column
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& ImGuiUtils::IsMouseInRectRegion({ rowAreaMin, rowAreaMax }, false)
			&& inSceneHierarchyTable)
		{
			SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());
		}

		if (isRowClicked && wasRowRightClicked)
		{
			ImGui::OpenPopup(rightClickPopupID.c_str());
		}

		// Drag/Drop...
		//if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		//{
		//	// Get the selected entities
		//	// then
		//	//ImGui::SetDragDropPayload("scene_entity_hierarchy_pay", &entity, sizeof(UUID));

		//	ImGui::EndDragDropSource();
		//}

		//if (ImGui::BeginDragDropTarget())
		//{
		//	const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("scene_entity_hierarchy_pay", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

		//	if (payLoad)
		//	{
		//		// Parent Entities...
		//	}
		//}

		if (opened)
		{
			// TODO: here we should display the child entites of a parent entity once parenting entites is a thing!
			ImGui::TableNextRow(0, rowHeight);
			ImGui::TableNextColumn();

			ImGuiUtils::ShiftCursorX(10.0f);
			ImGui::TextColored(ImVec4{ 0.4f, 0.8f, 0.4f, 1.0f }, "WIP! Until parenting entities is a thing!");

			ImGui::TreePop();
		}

		if (deleteEntity)
		{
			// Intentional Copy since DestroyEntity would call the EditorLayer::OnEntityDeleted which deselects the entity.
			std::vector<UUID> selectedEntites = SelectionManager::GetSelections(s_ActiveSelectionContext);
			for (UUID id : selectedEntites)
				m_Context->DestroyEntity(m_Context->GetEntityWithUUID(id));
		}
	}

	void SceneHierarchyPanel::DrawComponents(const std::vector<UUID>& entityIDs)
	{
		if (entityIDs.size() == 0)
			return;

		ImGuiFontsLibrary& fontLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;

		Entity firstEntity = m_Context->GetEntityWithUUID(entityIDs[0]);
		std::string& tag = firstEntity.GetName();

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
			ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "ID: %#010x", firstEntity.GetUUID());

		ImGui::SameLine();

		float lineHeight = ImGui::GetItemRectSize().y; // Get Height of last item

		//ImGuiUtils::ShiftCursorX(-5);
		if (ImGui::Button("Add Component", ImVec2{ ImGui::GetContentRegionAvail().x, lineHeight }))
			ImGui::OpenPopup("AddComponentsPopUp"); // AddComponentPopUp here acts as an ID for imgui to be used in the next step

		if (ImGui::BeginPopup("AddComponentsPopUp"))
		{
			Utils::DrawPopUpMenuEntry<CameraComponent, true>(this, "Camera");
			Utils::DrawPopUpMenuEntry<SpriteRendererComponent>(this, "Sprite Renderer");
			Utils::DrawPopUpMenuEntry<CircleRendererComponent>(this, "Circle Renderer");
			Utils::DrawPopUpMenuEntry<StaticMeshComponent, true>(this, "StaticMesh");
			Utils::DrawPopUpMenuEntry<DirectionalLightComponent>(this, "DirectionalLight");
			Utils::DrawPopUpMenuEntry<SkyLightComponent, true>(this, "SkyLight");
			Utils::DrawPopUpMenuEntry<TextComponent>(this, "Text");

			ImGui::EndPopup();
		}

		// TODO: DrawComponent needs to be changed so that it takes a vector of UUIDs to work nicely with RelationShipComponent when that exists!
		Utils::DrawComponent<TransformComponent>("Transform", firstEntity, [this](TransformComponent& component)
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

		Utils::DrawComponent<CameraComponent>("Camera", firstEntity, [](CameraComponent& component)
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

		Utils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", firstEntity, [](SpriteRendererComponent& component)
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

		Utils::DrawComponent<CircleRendererComponent>("Circle Renderer", firstEntity, [](CircleRendererComponent& component)
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

		// TODO: Remove once there is a drag drop
		static std::string s_EnvironmentMapName = "Aurora Default";

		Utils::DrawComponent<SkyLightComponent>("SkyLight", firstEntity, [](SkyLightComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGui::Text("Environment");

			ImGui::NextColumn();

			// TODO: Change to accept drag drop
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushStyleColor(ImGuiCol_Button, Theme::PropertyField);
			if (ImGui::Button(s_EnvironmentMapName.c_str(), { width, 25.0f }))
			{
				std::filesystem::path p = Utils::WindowsFileDialogs::OpenFileDialog("HDR Image (*.hdr)\0*.hdr\0");
				s_EnvironmentMapName = p.filename().string();

				if (s_EnvironmentMapName.size())
				{
					Ref<Asset> asset = AssetManager::GetAsset<Environment>(p);
					if (asset->IsValid())
						component.SceneEnvironment = asset->Handle;
				}
				else
					s_EnvironmentMapName = "Aurora Default";
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
			s_EnvironmentMapName = "Aurora Default";
			component.SceneEnvironment = 0;
			component.Level = 0.5f;
			component.Intensity = 1.0f;
			component.DynamicSky = false;
			component.TurbidityAzimuthInclination = { 2.0f, 0.0f, 0.0f };
		});

		// TODO: Remove once there is Drag/Drop
		static std::string s_StaticMeshName = "Aurora Default";

		Utils::DrawComponent<StaticMeshComponent>("StaticMesh", firstEntity, [](StaticMeshComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGui::Text("Static Mesh");

			ImGui::NextColumn();

			// TODO: Change to accept drag drop
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushStyleColor(ImGuiCol_Button, Theme::PropertyField);
			if (ImGui::Button(s_StaticMeshName.c_str(), { width, 25.0f }))
			{
				std::filesystem::path p = Utils::WindowsFileDialogs::OpenFileDialog("StaticMesh");
				s_StaticMeshName = p.filename().string();

				if (s_StaticMeshName.size())
				{
					Ref<Asset> asset = AssetManager::GetAsset<StaticMesh>(p);
					if (asset->IsValid())
					{
						component.StaticMesh = StaticMesh::Create(asset.As<MeshSource>())->Handle;
					}
				}
				else
					s_StaticMeshName = "Aurora Default";
			}
			ImGui::PopStyleColor();

			ImGuiUtils::EndPropertyGrid();

			//if (ImGui::Button("Static Mesh Path"))
			//{
			//	std::filesystem::path path = Utils::WindowsFileDialogs::OpenFileDialog("StaticMesh");
			//	Ref<MeshSource> meshSource = MeshSource::Create(path);
			//	component.StaticMesh = StaticMesh::Create(meshSource);
			//}
		},
		[](StaticMeshComponent& component)
		{
			s_StaticMeshName = "Aurora Default";
			component.StaticMesh = 0;
		});

		Utils::DrawComponent<DirectionalLightComponent>("DirectionalLight", firstEntity, [](DirectionalLightComponent& component)
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

		// TODO: Remove once there is a drag drop
		static std::string s_TextFileName = "Aurora Default";

		Utils::DrawComponent<TextComponent>("Text", firstEntity, [](TextComponent& component)
		{
			ImGuiUtils::BeginPropertyGrid();

			ImGui::Text("Font");

			ImGui::NextColumn();

			// TODO: Change to accept drag drop
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::PushStyleColor(ImGuiCol_Button, Theme::PropertyField);
			if (ImGui::Button(s_TextFileName.c_str(), { width, 25.0f }))
			{
				std::filesystem::path p = Utils::WindowsFileDialogs::OpenFileDialog("Font File (*.ttf)\0*.ttf\0");
				s_TextFileName = p.filename().string();

				if (s_TextFileName.size())
				{
					Ref<Asset> asset = AssetManager::GetAsset<Font>(p);
					if (asset->IsValid())
						component.FontHandle = asset->Handle;
				}
				else
					s_TextFileName = "Aurora Default";
			}
			ImGui::PopStyleColor();

			ImGui::NextColumn();

			if (ImGuiUtils::MultiLineText("Text", component.TextString))
			{
				component.TextHash = std::hash<std::string>()(component.TextString);
			}

			ImGui::Separator();

			ImGuiUtils::ColorEdit4Control("Color", component.Color);
			//ImGuiUtils::ColorEdit4Control("Outline Color", component.OutLineColor);
			ImGuiUtils::PropertyFloat("Line Spacing", component.LineSpacing, 0.05f, 0.0f, 30.0f);
			ImGuiUtils::PropertyFloat("Max Width", component.MaxWidth, 0.01f, 0.0f, 30.0f);
			ImGuiUtils::PropertyFloat("Kerning", component.Kerning, 0.01f, 0.0f, 30.0f);
			//ImGuiUtils::PropertyFloat("Outline Width", component.OutLineWidth, 0.1f, 0.0f, 2.0f);

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
			s_TextFileName = "Aurora Default";
		});
	}

	void SceneHierarchyPanel::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float colomnWidth, float min, float max, float stepValue)
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
		layer->m_FontsLibrary.PushTemporaryFont("RobotoBold", 17.0f);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, stepValue, min, max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		layer->m_FontsLibrary.PushTemporaryFont("RobotoBold", 17.0f);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, stepValue, min, max, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		layer->m_FontsLibrary.PushTemporaryFont("RobotoBold", 17.0f);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		layer->m_FontsLibrary.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, stepValue, min, max, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar(2);
		ImGui::Columns(1);

		ImGui::PopID();
	}

	void SceneHierarchyPanel::OnExternalEntityDestroyed(Entity entity)
	{
		m_EntityDeletedCallback(entity);
	}

}