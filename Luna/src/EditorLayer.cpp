#include "EditorLayer.h"

#include "Scene/Components.h"
#include "Scene/SceneSerializer.h"
#include "Utils/UtilFunctions.h"
#include "Utils/Math.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo/ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Aurora {

#pragma region EditorLayerMainMethods

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

		m_EditorScene = Scene::Create();
		m_ActiveScene = m_EditorScene;
		SetContextForSceneHeirarchyPanel(m_ActiveScene);

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

		//if (m_ViewPortFocused)
		//{
		m_EditorCamera.OnUpdate(ts);
		//}

		Renderer3D::ResetStats();

		m_Framebuffer->bind();
		RenderCommand::SetClearColor(m_Color);
		RenderCommand::Clear();

		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
		//m_ActiveScene->OnUpdateRuntime(ts);

		m_Framebuffer->unBind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>(AR_SET_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.IsRepeat())
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
		    case Key::N:
		    {
		    	if (control)
		    		NewScene();
		    
		    	break;
		    }

		    case Key::O:
		    {
		    	if (control)
		    		OpenScene();

				break;
		    }

		    case Key::S:
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

			// Gizmos
			case Key::Q:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;

				break;
			}

			case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;

				break;
			}

			case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;

				break;
			}

			case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;

				break;
			}
		}

		return true;
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
				m_SelectionContext = m_Context->CreateEntity(name.c_str());
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

#pragma endregion

	// NEED TO CHANGE SOME STUFF HERE FOR LATER
#pragma region FileDialogs/Scene Helpers

	void EditorLayer::NewScene()
	{
		m_ActiveScene = Scene::Create(); // Creating new scene
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		SetContextForSceneHeirarchyPanel(m_ActiveScene);
		m_NameCounter = 0;

		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		// TO BE CHANGED
		std::string filepath = Utils::WindowsFileDialogs::OpenFile("Aurora Scene (*.aurora)\0*.aurora\0");
		
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
		std::string filepath = Utils::WindowsFileDialogs::SaveFile("Aurora Scene (*.aurora)\0*.aurora\0");
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
		layer->m_Fonts.SetTemporaryFont("MochiyPopOne", FontIdentifier::Regular);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		layer->m_Fonts.PopTemporaryFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
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
		ImGui::DragFloat("##Y", &values.y, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
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
		ImGui::DragFloat("##Z", &values.z, stepValue, min, max, "%.2f");// I am setting the format to only display 2 decimal places however viable to change
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
				DrawVec3Control("Scale", component.Scale, 1.0f, 100.0f, 0.0f, 1000.0f, 0.025f);
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
		std::unordered_map<std::string, float>& mp = PerformanceTimer::GetTimeMap();

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
		ImGui::Text("V Sync: %s", Application::GetApp().GetWindow().IsVSync() ? "On" : "Off");
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

#pragma region HelpPanels/UI

	void EditorLayer::ShowEditorCameraHelpUI()
	{
		ImGui::Begin("Editor Camera Controls:", &m_ShowEditorCameraHelpUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("These are the editor camera's controls:");
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

			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Restart"))
				{
					// TODO: Add a panel that pops up if the person has an open scene that asks if they want to save before restarting
					Application::GetApp().Restart();
				}

				ImGui::Separator();

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
				if (ImGui::MenuItem("Performance", NULL, m_ShowPerformance)) m_ShowPerformance = !m_ShowPerformance;

				ImGui::Separator();

				if (ImGui::MenuItem("Renderer Info", NULL, m_ShowRendererVendorInfo)) m_ShowRendererVendorInfo = !m_ShowRendererVendorInfo;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Editor Camera", NULL, m_ShowEditorCameraHelpUI)) m_ShowEditorCameraHelpUI = !m_ShowEditorCameraHelpUI;

				ImGui::Separator();

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

		ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
		ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		ImVec2 viewportOffset = ImGui::GetWindowPos();

		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewPortFocused = ImGui::IsWindowFocused();
		m_ViewPortHovered = ImGui::IsWindowHovered();

		Application::GetApp().GetImGuiLayer()->SetBlockEvents(!m_ViewPortFocused && !m_ViewPortHovered);

		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = *(glm::vec2*)&viewPortPanelSize;

		uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
		ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Gizmos...
		if (m_SelectionContext && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);;

			// Camera

			// Editor Camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Runtime Camera
			//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//const glm::mat4& cameraProjection = camera.GetProjection();
			//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Entity transform
			auto& tc = m_SelectionContext.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			glm::mat4 iden(1.0f);
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation(0.0f), rotation(0.0f), scale(0.0f);
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation.x += deltaRotation.x;
				tc.Rotation.y += deltaRotation.y;
				tc.Rotation.z += deltaRotation.z;
				tc.Scale = scale;
			}
		}

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

		if (m_ShowEditorCameraHelpUI)
			ShowEditorCameraHelpUI();

		ShowViewport();
	}

}