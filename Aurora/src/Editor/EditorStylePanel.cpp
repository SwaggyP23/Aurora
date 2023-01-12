#include "Aurorapch.h"
#include "EditorStylePanel.h"

#include "Core/Application.h"
#include "ImGui/ImGuiUtils.h"
#include "Utils/YAMLSerializationHelpers.h"

namespace Aurora {

	// This matches the one in imgui_demo.cpp
	static const char* s_AvailableThemes[] = { "Luna", "Dark", "Light", "Classic" };
	static uint8_t s_DefaultThemeID = 0;

	namespace Utils {

	}

	Ref<EditorStylePanel> EditorStylePanel::Create()
	{
		return CreateRef<EditorStylePanel>();
	}

	void EditorStylePanel::OnImGuiRender(bool& isOpen)
	{
		ImGui::Begin("Editor Style (Close the panel in order to save changes...)", &isOpen);

		if (ImGuiUtils::PropertyGridHeader("Fonts"))
		{
			ShowFontPickerUI();

			ImGui::TreePop();
		}

		if (ImGuiUtils::PropertyGridHeader("Editor Style"))
		{
			ImGui::ShowStyleEditor(nullptr, (int*)&s_DefaultThemeID);

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void EditorStylePanel::Serialize(YAML::Emitter& mainEmitter)
	{
		mainEmitter << YAML::Key << "DefaultFont" << YAML::Value << Application::GetApp().GetImGuiLayer()->m_FontsLibrary.GetDefaultFontName();

		// TODO: Serialize all the ImGui style fields and the font that was picked...

		mainEmitter << YAML::Key << "StyleSettings";

		mainEmitter << YAML::BeginMap; // Style

		mainEmitter << YAML::Key << "Theme" << YAML::Value << s_AvailableThemes[s_DefaultThemeID];

		const ImGuiStyle& style = ImGui::GetStyle();
		mainEmitter << YAML::Key << "FrameRounding" << YAML::Value << style.FrameRounding;

		mainEmitter << YAML::Key << "WindowBorder" << YAML::Value << style.WindowBorderSize;
		mainEmitter << YAML::Key << "FrameBorder" << YAML::Value << style.FrameBorderSize;
		mainEmitter << YAML::Key << "PopupBorder" << YAML::Value << style.PopupBorderSize;

		// Sizes tab...

		mainEmitter << YAML::Key << "WindowPadding" << YAML::Value << style.WindowPadding;
		mainEmitter << YAML::Key << "FramePadding" << YAML::Value << style.FramePadding;
		mainEmitter << YAML::Key << "CellPadding" << YAML::Value << style.CellPadding;
		mainEmitter << YAML::Key << "ItemSpacing" << YAML::Value << style.ItemSpacing;
		mainEmitter << YAML::Key << "ItemInnerSpacing" << YAML::Value << style.ItemInnerSpacing;
		mainEmitter << YAML::Key << "TouchExtraPadding" << YAML::Value << style.TouchExtraPadding;
		mainEmitter << YAML::Key << "IndentSpacing" << YAML::Value << style.IndentSpacing;
		mainEmitter << YAML::Key << "ScrollbarSize" << YAML::Value << style.ScrollbarSize;
		mainEmitter << YAML::Key << "GrabMinSize" << YAML::Value << style.GrabMinSize;
		mainEmitter << YAML::Key << "WindowBorderSize" << YAML::Value << style.WindowBorderSize;
		mainEmitter << YAML::Key << "ChildBorderSize" << YAML::Value << style.ChildBorderSize;
		mainEmitter << YAML::Key << "PopupBorderSize" << YAML::Value << style.PopupBorderSize;
		mainEmitter << YAML::Key << "FrameBorderSize" << YAML::Value << style.FrameBorderSize;
		mainEmitter << YAML::Key << "TabBorderSize" << YAML::Value << style.TabBorderSize;
		mainEmitter << YAML::Key << "WindowRounding" << YAML::Value << style.WindowRounding;
		mainEmitter << YAML::Key << "ChildRounding" << YAML::Value << style.ChildRounding;
		mainEmitter << YAML::Key << "FrameRounding" << YAML::Value << style.FrameRounding;
		mainEmitter << YAML::Key << "PopupRounding" << YAML::Value << style.PopupRounding;
		mainEmitter << YAML::Key << "ScrollbarRounding" << YAML::Value << style.ScrollbarRounding;
		mainEmitter << YAML::Key << "GrabRounding" << YAML::Value << style.GrabRounding;
		mainEmitter << YAML::Key << "LogSliderDeadzone" << YAML::Value << style.LogSliderDeadzone;
		mainEmitter << YAML::Key << "TabRounding" << YAML::Value << style.TabRounding;
		mainEmitter << YAML::Key << "WindowTitleAlign" << YAML::Value << style.WindowTitleAlign;
		mainEmitter << YAML::Key << "WindowMenuButtonPosition" << YAML::Value << style.WindowMenuButtonPosition;
		mainEmitter << YAML::Key << "ColorButtonPosition" << YAML::Value << style.ColorButtonPosition;
		mainEmitter << YAML::Key << "ButtonTextAlign" << YAML::Value << style.ButtonTextAlign;
		mainEmitter << YAML::Key << "SelectableTextAlign" << YAML::Value << style.SelectableTextAlign;
		mainEmitter << YAML::Key << "DisplaySafeAreaPadding" << YAML::Value << style.DisplaySafeAreaPadding;

		// Rendering tab...

		mainEmitter << YAML::Key << "AntiAliasedLines" << YAML::Value << style.AntiAliasedLines;
		mainEmitter << YAML::Key << "AntiAliasedLinesUseTex" << YAML::Value << style.AntiAliasedLinesUseTex;
		mainEmitter << YAML::Key << "AntiAliasedFill" << YAML::Value << style.AntiAliasedFill;
		mainEmitter << YAML::Key << "CurveTessellationTol" << YAML::Value << style.CurveTessellationTol;
		mainEmitter << YAML::Key << "CircleTessellationMaxError" << YAML::Value << style.CircleTessellationMaxError;
		mainEmitter << YAML::Key << "GlobalAlpha" << YAML::Value << style.Alpha;
		mainEmitter << YAML::Key << "DisabledAlpha" << YAML::Value << style.DisabledAlpha;

		mainEmitter << YAML::EndMap; // Style
	}

	void EditorStylePanel::Deserialize(const YAML::Node& mainNode)
	{
		if (mainNode["DefaultFont"])
		{
			// Loading the defualt font...
			std::string defaultFont = mainNode["DefaultFont"].as<std::string>("RobotoDefault");
			Application::GetApp().GetImGuiLayer()->m_FontsLibrary.SetDefaultFont(defaultFont);
		}
		else
			AR_CONSOLE_LOG_WARN("Reverting to default font...");

		// TODO: Deserialize all the imgui style settings...

		if (!mainNode["StyleSettings"])
		{
			AR_CONSOLE_LOG_ERROR("[EditorPanelsLibrary]: Did not find a saved editor style... Reverting to default!");
			return;
		}

		YAML::Node styleSettingsNode = mainNode["StyleSettings"];
		if (!styleSettingsNode)
		{
			AR_CONSOLE_LOG_WARN("Did not find styleSettings node!");
			return;
		}

		std::string themeName = styleSettingsNode["Theme"].as<std::string>("Luna");
		
		for (uint8_t i = 0; i < 4; i++)
		{
			if (s_AvailableThemes[i] == themeName)
			{
				s_DefaultThemeID = i;
			}
		}

		switch (s_DefaultThemeID)
		{
			case 0: Application::GetApp().GetImGuiLayer()->SetDarkThemeColor(); break;
			case 1: ImGui::StyleColorsDark(); break;
			case 2: ImGui::StyleColorsLight(); break;
			case 3: ImGui::StyleColorsClassic(); break;
		}

		ImGuiStyle& style = ImGui::GetStyle();

		style.FrameRounding = styleSettingsNode["FrameRounding"].as<float>(2.0f);
		style.WindowBorderSize = styleSettingsNode["WindowBorder"].as<float>(1.0f);
		style.FrameBorderSize = styleSettingsNode["FrameBoder"].as<float>(1.0f);
		style.PopupBorderSize = styleSettingsNode["PopupBorder"].as<float>(1.0f);

		// Sizes tab...

		style.WindowPadding = styleSettingsNode["WindowPadding"].as<ImVec2>(ImVec2{ 8.0f, 8.0f });
		style.FramePadding = styleSettingsNode["FramePadding"].as<ImVec2>(ImVec2{ 4.0f, 4.0f });
		style.CellPadding = styleSettingsNode["CellPadding"].as<ImVec2>(ImVec2{ 4.0f, 2.0f });
		style.ItemSpacing = styleSettingsNode["ItemSpacing"].as<ImVec2>(ImVec2{ 8.0f, 8.0f });
		style.ItemInnerSpacing = styleSettingsNode["ItemInnerSpacing"].as<ImVec2>(ImVec2{ 4.0f, 4.0f });
		style.TouchExtraPadding = styleSettingsNode["TouchExtraPadding"].as<ImVec2>(ImVec2{ 0.0f, 0.0f });
		style.IndentSpacing = styleSettingsNode["IndentSpacing"].as<float>(11.0f);
		style.ScrollbarSize = styleSettingsNode["ScrollbarSize"].as<float>(14.0f);
		style.GrabMinSize = styleSettingsNode["GrabMinSize"].as<float>(12.0f);
		style.WindowBorderSize = styleSettingsNode["WindowBorderSize"].as<float>(1.0f);
		style.ChildBorderSize = styleSettingsNode["ChildBorderSize"].as<float>(1.0f);
		style.PopupBorderSize = styleSettingsNode["PopupBorderSize"].as<float>(1.0f);
		style.FrameBorderSize = styleSettingsNode["FrameBorderSize"].as<float>(1.0f);
		style.TabBorderSize = styleSettingsNode["TabBorderSize"].as<float>(0.0f);
		style.WindowRounding = styleSettingsNode["WindowRounding"].as<float>(6.0f);
		style.ChildRounding = styleSettingsNode["ChildRounding"].as<float>(0.0f);
		style.FrameRounding = styleSettingsNode["FrameRounding"].as<float>(2.0f);
		style.PopupRounding = styleSettingsNode["PopupRounding"].as<float>(3.0f);
		style.ScrollbarRounding = styleSettingsNode["ScrollbarRounding"].as<float>(9.0f);
		style.GrabRounding = styleSettingsNode["GrabRounding"].as<float>(0.0f);
		style.LogSliderDeadzone = styleSettingsNode["LogSliderDeadzone"].as<float>(4.0f);
		style.TabRounding = styleSettingsNode["TabRounding"].as<float>(4.0f);
		style.WindowTitleAlign = styleSettingsNode["WindowTitleAlign"].as<ImVec2>(ImVec2{ 0.0f, 0.5f });
		style.WindowMenuButtonPosition = styleSettingsNode["WindowMenuButtonPosition"].as<int>(0);
		style.ColorButtonPosition = styleSettingsNode["ColorButtonPosition"].as<int>(1);
		style.ButtonTextAlign = styleSettingsNode["ButtonTextAlign"].as<ImVec2>(ImVec2{ 0.5f, 0.5f });
		style.SelectableTextAlign = styleSettingsNode["SelectableTextAlign"].as<ImVec2>(ImVec2{ 0.0f, 0.0f });
		style.DisplaySafeAreaPadding = styleSettingsNode["DisplaySafeAreaPadding"].as<ImVec2>(ImVec2{ 3.0f, 3.0f });

		// Rendering tab...

		style.AntiAliasedLines = styleSettingsNode["AntiAliasedLines"].as<bool>(true);
		style.AntiAliasedLinesUseTex = styleSettingsNode["AntiAliasedLines"].as<bool>(true);
		style.AntiAliasedFill = styleSettingsNode["AntiAliasedLines"].as<bool>(true);
		style.CurveTessellationTol = styleSettingsNode["AntiAliasedLines"].as<float>(1.250f);
		style.CircleTessellationMaxError = styleSettingsNode["CircleTessellationMaxError"].as<float>(0.3f);
		style.Alpha = styleSettingsNode["GlobalAlpha"].as<float>(1.0f);
		style.DisabledAlpha = styleSettingsNode["DisabledAlpha"].as<float>(0.6f);
	}

	void EditorStylePanel::ShowFontPickerUI()
	{
		ImGuiFontsLibrary& fontsLib = Application::GetApp().GetImGuiLayer()->m_FontsLibrary;

		ImGuiUtils::BeginPropertyGrid();

		static const char* fonts[] = { "RobotoLarge", "RobotoBold", "RobotoDefault", "MochiyPopOne" };
		const char* selectedFont = fontsLib.GetDefaultFontName().c_str();

		static int selected = 2;
		for (uint8_t i = 0; i < 4; i++)
		{
			if (strcmp(fonts[i], selectedFont) == 0)
			{
				selected = i;
			}
		}

		if (ImGuiUtils::PropertyDropdown("Available Fonts", fonts, 4, &selected))
		{
			fontsLib.SetDefaultFont(fonts[selected]);
			AR_DEBUG("Selected int: {0}", selected);
		}

		ImGuiUtils::EndPropertyGrid();
	}

}