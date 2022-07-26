#include "EditorLayer.h"

#include "Scene/Components.h"
#include "Scene/SceneSerializer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Aurora {

	EditorLayer::EditorLayer()
		: Layer("BatchRenderer"),
		m_EditorCamera(EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f))
	{
	}

	void EditorLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = Scene::Create();
		SetContext(m_ActiveScene);

		class CameraScript : public ScriptableEntity
		{
			virtual void OnUpdate(TimeStep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				const float speed = 5.0f;

				if (Input::IsKeyPressed(Key::W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(Key::S))
					translation.y -= speed * ts;
				if (Input::IsKeyPressed(Key::A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(Key::D))
					translation.x += speed * ts;
			}
		};
	}

	void EditorLayer::OnDetach()
	{
		AR_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		AR_PROFILE_FUNCTION();
		AR_PERF_TIMER("EditorLayer::OnUpdate");

		// Framebuffer resizing... This stops the blacked out frames we would get when resizing...
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification(); m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f 
			&& (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (m_ViewPortFocused)
		{
			m_EditorCamera.OnUpdate(ts);
		}

		Renderer3D::ResetStats();

		m_Framebuffer->bind();
		RenderCommand::SetClearColor(m_Color);
		RenderCommand::Clear();

		m_ActiveScene->OnUpdate(ts);

		m_Framebuffer->unBind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
	}

#pragma region SceneHierarchyPanel

	void EditorLayer::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void EditorLayer::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false; // We mark the entity for deletion and then we delete it when the rendering finishes since there may be some ImGui code that needs to still run like popping
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{ // This is temporary...
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)1038597, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
			m_NameCounter--;
		}
	}

	void EditorLayer::ShowSceneHierarchyUI()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity{ entityID, m_Context.raw() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right clicking on a blank space menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity")) {
				// If the user forgot to change the tag of newly created entities, the editor will provide an incrementing number beside the default name for distinguishing them, however does not update on deletion
				std::string name = "Empty Entity " + std::to_string(m_NameCounter++);;
				m_Context->CreateEntity(name.c_str());
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

#pragma endregion

#pragma region ComponentsPanel

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			T& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.0f, 4.0f });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f; // From ImGui source
			ImGui::Separator();

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings"); // This is also just an id and not a tag that will be rendered
				}
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

	void EditorLayer::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float colomnWidth, float min, float max)
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
		layer->m_Fonts.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		layer->m_Fonts.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });		
		layer->m_Fonts.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		layer->m_Fonts.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });		
		layer->m_Fonts.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		layer->m_Fonts.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
		ImGui::PopItemWidth();

		ImGui::PopStyleVar(2);
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void EditorLayer::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputTextWithHint("##Tag", "Change entity name...", buffer, sizeof(buffer)))
				tag = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent"); // Add component here acts as an ID for imgui to be used in the next step

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (ImGui::MenuItem("Camera")) // This whole code here could be brought out to a templated function since the only vars are name and type
			{
				m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Sprite Renderer"))
			{
				m_SelectionContext.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [this](TransformComponent& component)
			{
				DrawVec3Control("Translation", component.Translation);
				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f, 100.0f, 0.0f, 1000.0f);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeString[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeString[(int)camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int type = 0; type < 2; type++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeString[type];
						if (ImGui::Selectable(projectionTypeString[type], isSelected))
						{
							currentProjectionTypeString = projectionTypeString[type];
							camera.SetProjectionType((SceneCamera::ProjectionType)type);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &verticalFOV, 0.1f))
						camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));

					float persNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &persNear, 0.1f))
						camera.SetPerspectiveNearClip(persNear);

					float persFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &persFar, 0.1f))
						camera.SetPerspectiveFarClip(persFar);
				}

				else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize, 0.1f))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear, 0.1f))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar, 0.1f))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		});
	}

	void EditorLayer::ShowComponentsUI()
	{
		ImGui::Begin("Properties");

		ImGui::ColorEdit3("Clear Color", (float*)&m_Color); //  This should be removed and that clear color is just cleared to black once we add environment maps and cubemaps and directional light
		ImGui::Separator();

		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}

		ImGui::End();
	}

#pragma endregion

#pragma region RendererPanels

	void EditorLayer::ShowRendererVendorInfoUI()
	{
		ImGui::Begin("Renderer Vendor", &m_ShowRendererVendorInfo, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Vendor: %s", RendererProperties::GetRendererProperties()->Vendor);
		ImGui::Text("Renderer: %s", RendererProperties::GetRendererProperties()->Renderer);
		ImGui::Text("OpenGL Version: %s", RendererProperties::GetRendererProperties()->Version);
		ImGui::Text("GLSL Version: %s", RendererProperties::GetRendererProperties()->GLSLVersion);
		ImGui::Text("Texture Slots Available: %d", RendererProperties::GetRendererProperties()->TextureSlots);

		ImGui::End();
	}

	void EditorLayer::ShowRendererStatsUI()
	{
		ImGui::Begin("Renderer Stats");

		float peak = std::max(m_Peak, ImGui::GetIO().Framerate);
		m_Peak = peak;
		ImGui::Text("Renderer Stats:");
		ImGui::Text("Draw Calls: %d", Renderer3D::GetStats().DrawCalls);
		ImGui::Text("Quad Count: %d", Renderer3D::GetStats().QuadCount);
		ImGui::Text("Vertex Count: %d", Renderer3D::GetStats().GetTotalVertexCount());
		ImGui::Text("Index Count: %d", Renderer3D::GetStats().GetTotalIndexCount());
		ImGui::Text("Vertex Buffer Usage: %.3f Megabytes", Renderer3D::GetStats().GetTotalVertexBufferMemory() / (1024.0f * 1024.0f));

		ImGui::End();
	}

#pragma endregion

#pragma region PerformancePanel

	static void ShowTimers()
	{
		auto& mp = PerformanceTimer::GetTimeMap();

		std::vector<std::pair<std::string, float>> timerValues;
		timerValues.reserve(mp.size());
		for (auto& [name, val] : mp)
			timerValues.emplace_back(name, val);

		std::sort(timerValues.begin(), timerValues.end(), [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) -> bool
			{
				return a.second > b.second;
			});

		for (auto& it : timerValues)
		{
			ImGui::Text("%.4f, %s", it.second, it.first.c_str());
		}
	}

	void EditorLayer::ShowPerformanceUI()
	{
		ImGui::Begin("Performance", &m_ShowPerformance);

		ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
		ImGui::Text("Peak FPS: %.f", m_Peak);
		ImGui::Text("CPU Frame: %.3f ms", Application::GetApp().GetCPUTime());
		ImGui::Text("Up Time: %.3f ms", Application::GetApp().GetTimeSinceStart());

		if (ImGui::TreeNodeEx("CPU Timers (Milliseconds)", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed))
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

		if (ImGui::TreeNodeEx("Panel Properties", flags))
		{
			ShowPanelPropertiesUI();

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
		sortedFontNames.reserve(layer->m_Fonts.GetFontNamesAndIdentifier().size());

		const char* idenType;
		std::string displayName;
		if (ImGui::BeginCombo("##FontPicker", m_SelectedFontName.c_str()))
		{
			for (const auto& it : layer->m_Fonts.GetFontNamesAndIdentifier())
			{
				switch (it.first.second)
				{
				case FontIdentifier::Bold:       idenType = ", Bold"; break;
				case FontIdentifier::Italic:     idenType = ", Italic"; break;
				case FontIdentifier::Regular:    idenType = ", Regular"; break;
				case FontIdentifier::Medium:     idenType = ", Medium"; break;
				case FontIdentifier::Light:      idenType = ", Light"; break;
				}

				displayName = it.first.first;
				displayName.append(idenType);
				sortedFontNames.emplace_back(displayName, it.first.first, it.first.second);
			}

			std::sort(sortedFontNames.begin(), sortedFontNames.end());
			for (const auto& [displayName, fontName, type] : sortedFontNames)
			{
				layer->m_Fonts.SetTemporaryFont(fontName, type);
				if (ImGui::Selectable(displayName.c_str()))
				{
					layer->m_Fonts.SetDefaultFont(fontName, type);
					m_SelectedFontName = displayName;
				}
				layer->m_Fonts.PopTemporaryFont();
			}

			ImGui::EndCombo();
		}
	}

#pragma endregion

#pragma region PanelPropertiesUI

	void EditorLayer::ShowPanelPropertiesUI()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		int window_menu_button_position = style.WindowMenuButtonPosition + 1;
		if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
			style.WindowMenuButtonPosition = window_menu_button_position - 1;

		if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
			style.GrabRounding = style.FrameRounding;

		ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");

		ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");

		ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");

		bool border = (style.WindowBorderSize > 0.0f);
		if (ImGui::Checkbox("WindowBorder", &border))
			style.WindowBorderSize = border ? 1.0f : 0.0f;
		
		ImGui::SameLine();
		border = (style.FrameBorderSize > 0.0f);
		if (ImGui::Checkbox("FrameBorder", &border))
			style.FrameBorderSize = border ? 1.0f : 0.0f;

		ImGui::SameLine();
		border = (style.PopupBorderSize > 0.0f);
		if (ImGui::Checkbox("PopupBorder", &border))
			style.PopupBorderSize = border ? 1.0f : 0.0f;

		border = (style.TabBorderSize > 0.0f);
		if (ImGui::Checkbox("TabBorder", &border))
			style.TabBorderSize = border ? 1.0f : 0.0f;
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
		style.WindowMinSize.x = 370.0f; // try resetting it to 340 however to do that i will need to add a column between the tag and add component button
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
				if (ImGui::MenuItem("Serialize")) // TODO: Change these to their own function in case in the future somethings need to be added and it would look better
				{
					SceneSerializer serialize(m_ActiveScene);
					serialize.SerializeToText("resources/scenes/editor.aurora");
				}

				if (ImGui::MenuItem("Deserialize"))
				{
					SceneSerializer serialize(m_ActiveScene);
					serialize.DeSerializeFromText("resources/scenes/editor.aurora");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				if (ImGui::MenuItem("Performance", NULL, m_ShowPerformance)) m_ShowPerformance = !m_ShowPerformance;
				if (ImGui::MenuItem("Restart")) Application::GetApp().Restart();
				if (ImGui::MenuItem("Exit")) Application::GetApp().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Editor Style", NULL, m_ShowEditingPanel)) m_ShowEditingPanel = !m_ShowEditingPanel;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Info"))
			{
				if (ImGui::MenuItem("Renderer Info", NULL, m_ShowRendererVendorInfo)) m_ShowRendererVendorInfo = !m_ShowRendererVendorInfo;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Dear ImGui Demo", NULL, m_ShowDearImGuiDemoWindow)) m_ShowDearImGuiDemoWindow = !m_ShowDearImGuiDemoWindow;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::ShowViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

		m_ViewPortFocused = ImGui::IsWindowFocused();
		m_ViewPortHovered = ImGui::IsWindowHovered();

		// If viewport is not focused OR is not hovered -> Block events
		// Which means if we lost focus however we are still hovered, that is not acceptable -> Block events
		Application::GetApp().GetImGuiLayer()->SetBlockEvents(!m_ViewPortFocused || !m_ViewPortHovered);

		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = *(glm::vec2*)&viewPortPanelSize;

		uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
		ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

#pragma endregion

	void EditorLayer::OnImGuiRender()
	{
		AR_PROFILE_FUNCTION();

		EnableDocking();

		ShowMenuBarItems();

		if (m_ShowDearImGuiDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDearImGuiDemoWindow);

		ShowSceneHierarchyUI();

		ShowComponentsUI();

		if(m_ShowRendererVendorInfo)
			ShowRendererVendorInfoUI();

		ShowRendererStatsUI();

		if (m_ShowPerformance)
			ShowPerformanceUI();

		if (m_ShowEditingPanel)
			ShowEditPanelUI();

		ShowViewport();
	}

}