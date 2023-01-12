#include "Aurorapch.h"
#include "ApplicationSettingsPanel.h"

#include "Core/Application.h"
#include "ImGui/ImGuizmo.h"
#include "ImGui/ImGuiUtils.h"

#include <yaml-cpp/yaml.h>

/*
 * The Blending/Culling/DepthTesting global states will not be serialized since opengl is stupid with its global state and this will not exist with
 * when vulkan is around...
 */

namespace Aurora{

	// None owning pointer just to access the show grid option...
	SceneRenderer* s_ViewportRenderer = nullptr;
	bool s_ShowGizmos = true;
	bool s_ShowBoundingBoxes = false;
	bool s_ShowIcons = true;
	bool s_AllowGizmoAxisFlip = true;

	Ref<ApplicationSettingsPanel> ApplicationSettingsPanel::Create(SceneRenderer* viewportRenderer)
	{
		return CreateRef<ApplicationSettingsPanel>(viewportRenderer);
	}

	ApplicationSettingsPanel::ApplicationSettingsPanel(SceneRenderer* viewportRenderer)
	{
		s_ViewportRenderer = viewportRenderer;
	}

	// TODO: The culling/depth/blend state will be removed when switching to vulkan since it does not have this much global state!
	void ApplicationSettingsPanel::OnImGuiRender(bool& isOpen)
	{
		static bool enableCulling = true;
		static bool enableBlending = true;
		static bool enableDepthTesting = true;

		static std::string cullOption = "Back";
		static std::string depthOption = "Less";

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

		ImGui::Begin("Settings (Close the panel in order to save changes...)", &isOpen);

		ImGuiUtils::BeginPropertyGrid(2, false);

		float tickDelta = Application::GetApp().GetTickDeltaTime();
		if (ImGuiUtils::PropertySliderFloat("App Tick Delta", tickDelta, 0.0f, 5.0f, "%.3f", "Controls the delta that it takes to call the OnTick function"))
			Application::GetApp().SetTickDeltaTime(tickDelta);

		ImGuiUtils::PropertyBool("Show Gizmos", s_ShowGizmos);
		ImGuiUtils::PropertyBool("Show Grid", s_ViewportRenderer->GetOptions().ShowGrid);
		ImGuiUtils::PropertyBool("Show Bounding Boxes", s_ShowBoundingBoxes);
		ImGuiUtils::PropertyBool("Show Icons", s_ShowIcons);
		if (ImGuiUtils::PropertyBool("Allow gizmo flip", s_AllowGizmoAxisFlip))
			ImGuizmo::AllowAxisFlip(s_AllowGizmoAxisFlip);

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

			// NOTE: All this shit down here will not be serialized since as the note said up top that this shit will not exist with vulkan!!!
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

	bool ApplicationSettingsPanel::ShowGizmos() const
	{
		return s_ShowGizmos;
	}

	bool ApplicationSettingsPanel::ShowBoundingBoxes() const
	{
		return s_ShowBoundingBoxes;
	}

	bool ApplicationSettingsPanel::ShowIcons() const
	{
		return s_ShowIcons;
	}

	bool ApplicationSettingsPanel::ShowGrid() const
	{
		return s_ViewportRenderer->GetOptions().ShowGrid;
	}

	bool ApplicationSettingsPanel::AllowGizmoFlip() const
	{
		return s_AllowGizmoAxisFlip;
	}

	void ApplicationSettingsPanel::Serialize(YAML::Emitter& mainEmitter)
	{
		mainEmitter << YAML::Key << "ApplicationSettings";

		RendererConfig& config = Renderer::GetConfig();

		mainEmitter << YAML::BeginMap; // ApplicationSettings

		mainEmitter << YAML::Key << "AppTickDelta" << YAML::Value << Application::GetApp().GetTickDeltaTime();
		mainEmitter << YAML::Key << "ShowGizmos" << YAML::Value << s_ShowGizmos;
		mainEmitter << YAML::Key << "ShowIcons" << YAML::Value << s_ShowIcons;
		mainEmitter << YAML::Key << "ShowBoundingBoxes" << YAML::Value << s_ShowBoundingBoxes;
		mainEmitter << YAML::Key << "ShowGrid" << YAML::Value << s_ViewportRenderer->GetOptions().ShowGrid;
		mainEmitter << YAML::Key << "AllowGizmoAxisFlip" << YAML::Value << s_AllowGizmoAxisFlip;
		mainEmitter << YAML::Key << "EnvironmentMapSize" << YAML::Value << config.EnvironmentMapResolution;
		mainEmitter << YAML::Key << "IrradianceMapComputeSamples" << YAML::Value << config.IrradianceMapComputeSamples;

		mainEmitter << YAML::EndMap; // ApplicationSettings
	}

	void ApplicationSettingsPanel::Deserialize(const YAML::Node& mainNode)
	{
		if (!mainNode["ApplicationSettings"])
		{
			AR_CONSOLE_LOG_ERROR("Loading default Application settings...");
			return;
		}

		YAML::Node settings = mainNode["ApplicationSettings"];
		RendererConfig& config = Renderer::GetConfig();

		float tickDelta = settings["AppTickDelta"].as<float>(1.0f);
		Application::GetApp().SetTickDeltaTime(tickDelta);
		s_ShowGizmos = settings["ShowGizmos"].as<bool>(true);
		s_ShowIcons = settings["ShowIcons"].as<bool>(true);
		s_ShowBoundingBoxes = settings["ShowBoundingBoxes"].as<bool>(false);
		s_ViewportRenderer->GetOptions().ShowGrid = settings["ShowGrid"].as<bool>(true);
		s_AllowGizmoAxisFlip = settings["AllowGizmoAxisFlip"].as<bool>(true);
		config.EnvironmentMapResolution = settings["EnvironmentMapSize"].as<uint32_t>(1024);
		config.IrradianceMapComputeSamples = settings["IrradianceMapComputeSamples"].as<uint32_t>(1024);
	}

}