#include "EditorLayer.h"

#include "ImGui/ImGuiUtils.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Aurora {



#pragma region EditorLayerMainMethods

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_EditorCamera(EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 10000.0f))
	{
	}

	static glm::vec3 s_MaterialAlbedoColor(1.0f);

	void EditorLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = Scene::Create("Editor Scene");
		m_ActiveScene = m_EditorScene;
		SetContextForSceneHeirarchyPanel(m_ActiveScene);

		auto plane = m_ActiveScene->CreateEntity();
		m_GroundEntity = plane;
		auto& name = m_GroundEntity.GetComponent<TagComponent>();
		name.Tag = "Floor plane";
		auto& tc = m_GroundEntity.GetComponent<TransformComponent>();
		tc.Scale = glm::vec3{ 200.0f, 0.5f, 200.0f };
		auto& sp = m_GroundEntity.AddComponent<SpriteRendererComponent>();
		sp.Color = { 0.6f, 0.6f, 0.6f, 1.0f };


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
	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		AR_PROFILE_FUNCTION();
		AR_SCOPE_PERF("EditorLayer::OnUpdate");

		// Framebuffer resizing... This stops the blacked out frames we would get when resizing...
		FramebufferSpecification spec = m_Framebuffer->GetSpecification();
		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_EditorCamera.SetActive(m_AllowViewportCameraEvents || Input::GetCursorMode() == CursorMode::Locked);
		m_EditorCamera.OnUpdate(ts);

		Renderer3D::ResetStats();

		m_Framebuffer->Bind();
		RenderCommand::SetClearColor(glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f }); // TODO: Framebuffers should contain the clear colors and not called like that, and then maybe clear is called in begin scene
		RenderCommand::Clear();
		m_Framebuffer->ClearTextureAttachment(1, -1);

		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera, s_MaterialAlbedoColor);
		//m_ActiveScene->OnUpdateRuntime(ts);

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			// This is a very slow operation, and itself adds about 1-2 milliseconds to our CPU Frame, however this is just for the editor so...
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY); // Index 1 since the second buffer is the RED_INTEGER buffer
			m_HoveredEntity = pixelData == -1 ? Entity::nullEntity : Entity{ (entt::entity)pixelData, m_ActiveScene.raw()};
		}
		
		m_Framebuffer->UnBind();
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

			bool isSomethingSelected = m_SelectionContext ? true : false;

			switch (e.GetKeyCode())
			{
			    case AR_KEY_N:
			    {
			    	if (control)
			    		NewScene();
			    
			    	break;
			    }
			    
			    case AR_KEY_O:
			    {
			    	if (control)
			    		OpenScene();
			    
			    	break;
			    }
			    
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
			    
			    case AR_KEY_D:
			    {
			    	// TODO: Needs rework...
			    	if (control && isSomethingSelected)
			    	{
			    		m_SelectionContext = m_ActiveScene->CopyEntity(m_SelectionContext);
			    	}
			    
			    	break;
			    }
			    
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
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (!ImGuizmo::IsOver() && !Input::IsKeyPressed(AR_KEY_LEFT_ALT))
		{
			m_SelectionContext = m_HoveredEntity;
			if (m_HoveredEntity != Entity::nullEntity)
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

	void EditorLayer::DrawEntityNode(Entity entity)
	{
		const std::string& tag = entity.GetComponent<TagComponent>().Tag;

		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{ 1.0f, 1.0f, 0.529f, 0.235f });
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		ImGui::PopStyleColor();

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
			bool opened2 = ImGui::TreeNodeEx((void*)1038597, flags, tag.c_str());
			if (opened2)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
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

		if (Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_LEFT) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// Right clicking on a blank space menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity")) {
				// If the user forgot to change the tag of newly created entities, the editor will provide an incrementing number beside the default name for distinguishing them, however does not update on deletion
				m_SelectionContext = m_Context->CreateEntity("Empty Entity");
			}

			ImGui::EndPopup();
		}

		ImGui::End();
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

		m_EditorScenePath = std::filesystem::path();
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

#pragma region ComponentsPanel

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

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				if (ImGui::Button("X", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings"); // This is also just an id and not a tag that will be rendered
				}

				ImGui::SameLine((float)(contentRegionAvail.x - 1.5 * lineHeight));
				if (ImGui::Button("R", ImVec2{ lineHeight, lineHeight }))
				{
					resetFunction(component);
				}
			}
			else
			{
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				if (ImGui::Button("R", ImVec2{ lineHeight, lineHeight }))
				{
					resetFunction(component);
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
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputTextWithHint("##Tag", "Change entity name...", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			if (ImGui::IsItemHovered())
				ImGuiUtils::ToolTipWithVariableArgs(ImVec4{ 1.0f, 1.0f, 0.529f, 0.7f }, "ID: %#010x", 12387519348766); // TODO: Switch to UUID when implemented
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponentsPanel"); // Add component here acts as an ID for imgui to be used in the next step

		if (ImGui::BeginPopup("AddComponentsPanel"))
		{
			DrawPopUpMenuItems<CameraComponent>("Camera", entity, m_SelectionContext);
			DrawPopUpMenuItems<SpriteRendererComponent>("Sprite Renderer", entity, m_SelectionContext);
			DrawPopUpMenuItems<ModelComponent>("Model Component", entity, m_SelectionContext);

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
		},
		[](SpriteRendererComponent& component) // Reset Function
		{
			glm::vec4& color = component.Color;
			color = glm::vec4(1.0f);
		});
	}

	void EditorLayer::ShowComponentsUI()
	{
		ImGui::Begin("Properties");

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

		std::string name = "None";
		if (m_HoveredEntity)
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;

		ImGui::Text("Hovered Entity: %s", name.c_str());
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

		for (Ref<Shader> shader : Shader::s_AllShaders)
		{
			const std::string& shaderName = shader->GetName();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			if(ImGui::TreeNodeEx(shaderName.c_str(), flags))
			{
				ImGui::Text("Path: %s", shader->GetFilePath().c_str());
				if (ImGui::Button("Reload"))
					shader->Reload(true);

				ImGui::TreePop();
			}

			ImGui::Separator();
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

		ImGui::ColorEdit3("Material Tint", glm::value_ptr(s_MaterialAlbedoColor));

		// TODO: TEMPORARY!!!!!!!!!!
		static float TickDelta = 1.0f;
		if (ImGui::SliderFloat("aoiuh", &TickDelta, 0.0f, 3.0f))
			Application::GetApp().SetTickDeltaTime(TickDelta);

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

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Shaders...", NULL, m_ShowShadersPanel))
					m_ShowShadersPanel = !m_ShowShadersPanel;

				ImGui::Separator();

				if (ImGui::MenuItem("Performance", NULL, m_ShowPerformance)) 
					m_ShowPerformance = !m_ShowPerformance;

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

				if (ImGui::MenuItem("Restart...", NULL, m_ShowRestartModal))
					m_ShowRestartModal = !m_ShowRestartModal;

				ImGui::Separator();

				if (ImGui::MenuItem("Exit...", NULL, m_ShowCloseModal))
					m_ShowCloseModal = !m_ShowCloseModal;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Editor Style", NULL, m_ShowEditingPanel))
					m_ShowEditingPanel = !m_ShowEditingPanel;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Editor Camera", NULL, m_ShowEditorCameraHelpUI))
					m_ShowEditorCameraHelpUI = !m_ShowEditorCameraHelpUI;

				ImGui::Separator();

				if (ImGui::MenuItem("Dear ImGui Demo", NULL, m_ShowDearImGuiDemoWindow))
					m_ShowDearImGuiDemoWindow = !m_ShowDearImGuiDemoWindow;

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

		glm::mat4 iden(1.0f);
		ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
			nullptr, snap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing() && !Input::IsKeyPressed(AR_KEY_LEFT_ALT))
		{
			glm::vec3 translation(0.0f), rotation(0.0f), scale(0.0f);
			Math::DecomposeTransform(transform, translation, rotation, scale);

			glm::vec3 deltaRotation = rotation - tc.Rotation;
			tc.Translation = translation;
			tc.Rotation += deltaRotation;
			tc.Scale = scale;
		}
	}

	void EditorLayer::ShowViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration);

		m_ViewPortFocused = ImGui::IsWindowFocused();
		m_ViewPortHovered = ImGui::IsWindowHovered();

		auto viewportOffset = ImGui::GetCursorPos();
		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = *(glm::vec2*)&viewPortPanelSize;

		uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
		ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);

		Application::GetApp().GetImGuiLayer()->SetBlockEvents(!m_ViewPortFocused || !m_ViewPortHovered);

		// Gizmos...
		if (m_SelectionContext && m_GizmoType != -1)
		{
			float rw = (float)ImGui::GetWindowWidth();
			float rh = (float)ImGui::GetWindowHeight();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

			ManipulateGizmos();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	template<typename UIFunction>
	static void DrawSettingsFeatureCheckbox(const std::string& name, const std::string& description, bool* controller, FeatureControl feature, UIFunction func)
	{
		ImGui::Text((name + ":").c_str());
		ImGui::SameLine();
		ImGuiUtils::ShowHelpMarker(description.c_str());

		ImGui::Checkbox(("##" + name).c_str(), controller);
		ImGui::SameLine();

		if (!(*controller))
			RenderCommand::Disable(feature);
		else
			RenderCommand::Enable(feature);

		func();
	}

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

		ImGui::Begin("Settings", &m_ShowSettingsUI);

		std::string blendDesc = "Blending is the technique that allows and implements the \"Transparency\" within objects.";
		DrawSettingsFeatureCheckbox("Blending", blendDesc, &enableBlending, FeatureControl::Blending, []()
		{
			if (ImGui::BeginCombo("Blending Function", blendOption.c_str()))
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

				ImGui::EndCombo();
			}
		});

		// Blend equation
		ImGui::Indent(32.5f);
		if (ImGui::BeginCombo("Blending Equation", blendEquation.c_str())) // TODO: Fix the naming to display in the combo label
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

			ImGui::EndCombo();
		}
		ImGui::Unindent(32.5f);

		std::string cullingDesc = "Culling is used to specify to the renderer what face is not to be processed thus reducing processing time.";
		DrawSettingsFeatureCheckbox("Culling", cullingDesc, &enableCulling, FeatureControl::Culling, []()
		{
			if (ImGui::BeginCombo("Culling Function", cullOption.c_str())) // TODO: Fix the naming to display in the combo label
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
		});

		std::string depthDesc = "Depth testing is what allows for 3D objects to appear on the screen via a depth buffer.";
		DrawSettingsFeatureCheckbox("DepthTesting", depthDesc, &enableDepthTesting, FeatureControl::DepthTesting, []()
		{
			if (ImGui::BeginCombo("Depth Function", depthOption.c_str()))
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
		});

		ImGui::End();
	}
	
	void EditorLayer::ShowRestartModalUI()
	{
		ImGui::OpenPopup("Restart?");

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Restart?", &m_ShowRestartModal, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextColored(ImVec4{ 0.7f, 0.4f, 0.3f, 1.0f }, "Are you sure...?");
			ImGui::Text("Make sure you saved all your files and \nscenes before restarting!\n\n");

			ImGui::Separator();

			if (ImGui::Button("Restart", ImVec2(120, 0)))
				Application::GetApp().Restart();

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				m_ShowRestartModal = false;
			}

			ImGui::EndPopup();
		}
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

		ShowMenuBarItems();

		if (m_ShowDearImGuiDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDearImGuiDemoWindow);

		ShowSceneHierarchyUI();

		ShowComponentsUI();

		if(m_ShowRendererVendorInfo)
			ShowRendererVendorInfoUI();

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

		if (m_ShowRestartModal)
			ShowRestartModalUI();

		if (m_ShowCloseModal)
			ShowCloseModalUI();

		ShowViewport();
	}

}