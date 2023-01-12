#include "Aurorapch.h"
#include "FontsLibrary.h"

#include <imgui/imgui.h>

namespace Aurora {

	void ImGuiFontsLibrary::SetDefaultFont(const std::string& fontName)
	{
		if (m_Fonts.find(fontName) != m_Fonts.end())
		{
			ImGuiIO& io = ImGui::GetIO();
			io.FontDefault = m_Fonts.at(fontName);
			m_DefaultFont = fontName;
			return;
		}

		AR_CORE_ASSERT(false);
	}

	void ImGuiFontsLibrary::PushTemporaryFont(const std::string& fontName, float size)
	{
		AR_CORE_ASSERT(!m_PushedTemporaryFont);
		m_PushedTemporaryFont = true;

		if (m_Fonts.find(fontName) != m_Fonts.end())
		{
			ImFont* font = m_Fonts.at(fontName);
			float fontSize = font->FontSize;
			if(size != 0.0f)
				font->FontSize = size;
			ImGui::PushFont(m_Fonts.at(fontName));
			font->FontSize = fontSize;

			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		ImGui::PushFont(io.FontDefault);
	}

	void ImGuiFontsLibrary::PopTemporaryFont()
	{
		AR_CORE_ASSERT(m_PushedTemporaryFont);
		m_PushedTemporaryFont = false;

		ImGui::PopFont();
	}

	void ImGuiFontsLibrary::LoadFont(const FontSpecification& spec, bool default)
	{
		if (m_Fonts.find(spec.FontName) != m_Fonts.end())
		{
			AR_CORE_WARN_TAG("ImGuiFontsLibrary", "Tried to add font with the same name {0}", spec.FontName);
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig config = {};
		config.MergeMode = spec.MergeWithLast;
		ImFont* font = io.Fonts->AddFontFromFileTTF(spec.FontFilePath.c_str(), spec.Size, &config, spec.GlyphRanges == nullptr ? io.Fonts->GetGlyphRangesDefault() : spec.GlyphRanges);
		AR_CORE_CHECK(font, "Font failed to load");
		m_Fonts[spec.FontName] = font;

		if (default)
		{
			io.FontDefault = font;
			m_DefaultFont = spec.FontName;
		}
	}

	ImFont* ImGuiFontsLibrary::GetFont(const std::string& fontName)
	{
		if (m_Fonts.find(fontName) != m_Fonts.end())
		{
			return m_Fonts.at(fontName);
		}

		AR_CORE_ASSERT(false);
		return nullptr;
	}

}