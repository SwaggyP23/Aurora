#include "Aurorapch.h"
#include "FontsLibrary.h"

#include <imgui/imgui.h>

namespace Aurora {

	void FontsLibrary::SetDefaultFont(const std::string& fontName, FontIdentifier type, float fontSize)
	{
		AR_PROFILE_FUNCTION();

#if AURORA_DEBUG
		std::string FontType;
		switch (type)
		{
		    case FontIdentifier::Bold:       FontType = GetFont(fontName).Bold; break;
		    case FontIdentifier::Italic:     FontType = GetFont(fontName).Italic; break;
		    case FontIdentifier::Regular:    FontType = GetFont(fontName).Regular; break;
		    case FontIdentifier::Medium:     FontType = GetFont(fontName).Medium; break;
		    case FontIdentifier::Light:      FontType = GetFont(fontName).Light; break;
		}
#endif

		AR_CORE_ASSERT(FontType != m_ErrorToken, "[FontsLibrary]: Font is not supported!");

		AddFont(fontName, type);
		
		ImGuiIO& io = ImGui::GetIO();
		io.FontDefault = io.Fonts->Fonts[m_FontIndices[{ fontName, type }]];
	}

	void FontsLibrary::SetTemporaryFont(const std::string& fontName, FontIdentifier type)
	{
		AR_PROFILE_FUNCTION();

#if AURORA_DEBUG
		std::string FontType;
		switch (type)
		{
		    case FontIdentifier::Bold:       FontType = GetFont(fontName).Bold; break;
        	case FontIdentifier::Italic:     FontType = GetFont(fontName).Italic; break;
        	case FontIdentifier::Regular:    FontType = GetFont(fontName).Regular; break;
        	case FontIdentifier::Medium:     FontType = GetFont(fontName).Medium; break;
        	case FontIdentifier::Light:      FontType = GetFont(fontName).Light; break;
		}
#endif

		AR_CORE_ASSERT(FontType != m_ErrorToken, "[FontsLibrary]: Font is not supported!"); // Check the temporary Font and the FontIdentifier that you pushed!

		std::pair<std::string, FontIdentifier> fontQualifier = std::make_pair(fontName, type);

		if (m_FontIndices.find(fontQualifier) != m_FontIndices.end())
		{
			ImGuiIO& io = ImGui::GetIO();

			ImFont* fontToPush = io.Fonts->Fonts[m_FontIndices[fontQualifier]];
			ImGui::PushFont(fontToPush);
			m_PushedTemporaryFont = true;
		}
	}

	void FontsLibrary::PopTemporaryFont()
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(m_PushedTemporaryFont, "[FontsLibrary]: No temporary font has been pushed, so this function is called in the wrong place or should not be called!")

		ImGui::PopFont();
		m_PushedTemporaryFont = false;
	}

	void FontsLibrary::AddFont(const std::string& fontName, FontIdentifier type, float fontSize)
	{
		AR_PROFILE_FUNCTION();

#if AURORA_DEBUG
		std::string FontType;
		switch (type)
		{
		    case FontIdentifier::Bold:       FontType = GetFont(fontName).Bold; break;
		    case FontIdentifier::Italic:     FontType = GetFont(fontName).Italic; break;
		    case FontIdentifier::Regular:    FontType = GetFont(fontName).Regular; break;
		    case FontIdentifier::Medium:     FontType = GetFont(fontName).Medium; break;
		    case FontIdentifier::Light:      FontType = GetFont(fontName).Light; break;
		}
#endif

		AR_CORE_ASSERT(FontType != m_ErrorToken, "[FontsLibrary]: Font is not supported!");

		ImGuiIO& io = ImGui::GetIO();
		std::string path = m_Directories[fontName][type].string();

		std::pair<std::string, FontIdentifier> fontQualifier = std::make_pair(fontName, type);

		// This caches the indices for the fonts for imgui and it also protects against loading the same font twice
		if (m_FontIndices.find(fontQualifier) == m_FontIndices.end())
		{
			io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize);
			m_FontIndices[fontQualifier] = m_Incrementor++;
		}
	}

	void FontsLibrary::LoadFont(const std::string& fontName, const std::string& directory)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(m_Directories.find(fontName) == m_Directories.end(), "[FontsLibrary]: Font already added!");

		for (const auto& it : std::filesystem::directory_iterator{ directory })
		{
			std::string entryStringNmae = it.path().string();
			std::string_view entryName = entryStringNmae.c_str();

			size_t lastSlash = entryName.find_last_of("/\\");
			lastSlash = lastSlash == std::string_view::npos ? 0 : lastSlash + 1;

			size_t lastPeriod = entryName.rfind('.');
			size_t count = lastPeriod == std::string_view::npos ? entryName.size() - lastSlash : lastPeriod - lastSlash;
			std::string_view Name = entryName.substr(lastSlash, count);

			std::string_view extension = entryName.substr(lastPeriod, lastPeriod + 2);

			if (extension == ".ttf")
			{
			
				size_t pos = Name.find("-Bold");
				if (pos != std::string::npos)
				{
					m_Directories[fontName][FontIdentifier::Bold] = it.path();
					m_BoldHandled = true;
					continue;
				}

				pos = Name.find("-Italic");
				if (pos != std::string::npos)
				{
					m_Directories[fontName][FontIdentifier::Italic] = it.path();
					m_ItalicHandled = true;
					continue;
				}
				
				pos = Name.find("-Regular");
				if (pos != std::string::npos)
				{
					m_Directories[fontName][FontIdentifier::Regular] = it.path();
					m_RegularHandled = true;
					continue;
				}

				pos = Name.find("-Medium");
				if (pos != std::string::npos)
				{
					m_Directories[fontName][FontIdentifier::Medium] = it.path();
					m_MediumHandled = true;
					continue;
				}

				pos = Name.find("-Light");
				if (pos != std::string::npos)
				{
					m_Directories[fontName][FontIdentifier::Light] = it.path();
					m_LightHandled = true;
					continue;
				}
			}
		}

		if (!m_BoldHandled)
			m_Directories[fontName][FontIdentifier::Bold] = m_ErrorToken;

		if (!m_ItalicHandled)
			m_Directories[fontName][FontIdentifier::Italic] = m_ErrorToken;

		if (!m_RegularHandled)
			m_Directories[fontName][FontIdentifier::Regular] = m_ErrorToken;

		if (!m_MediumHandled)
			m_Directories[fontName][FontIdentifier::Medium] = m_ErrorToken;

		if (!m_LightHandled)
			m_Directories[fontName][FontIdentifier::Light] = m_ErrorToken;

		// Reset the flags so that next time we load another font we do the final checks
		m_BoldHandled = false;
		m_ItalicHandled = false;
		m_RegularHandled = false;
		m_MediumHandled = false;
		m_LightHandled = false;
	}

	FontTypes FontsLibrary::GetFont(const std::string& fontName)
	{
		AR_PROFILE_FUNCTION();

		FontTypes res;
		
		AR_CORE_ASSERT(m_Directories.find(fontName) != m_Directories.end(), "This font is not added to the library!");

		res.Bold = m_Directories[fontName][FontIdentifier::Bold].string();
		res.Italic = m_Directories[fontName][FontIdentifier::Italic].string();
		res.Regular = m_Directories[fontName][FontIdentifier::Regular].string();
		res.Medium = m_Directories[fontName][FontIdentifier::Medium].string();
		res.Light = m_Directories[fontName][FontIdentifier::Light].string();

		return res;
	}

}