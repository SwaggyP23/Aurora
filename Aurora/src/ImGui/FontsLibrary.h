#pragma once

#include <unordered_map>

typedef unsigned short ImWchar;
struct ImFont;

namespace Aurora {

	struct FontSpecification
	{
		std::string FontName;
		std::string FontFilePath;
		float Size = 18.0f;
		const ImWchar* GlyphRanges = nullptr;
		bool MergeWithLast = false;
	};

	class ImGuiFontsLibrary
	{
	public:
		ImGuiFontsLibrary() = default;
		~ImGuiFontsLibrary() = default;

		void SetDefaultFont(const std::string& fontName);
		void PushTemporaryFont(const std::string& fontName, float size = 0.0f);
		void PopTemporaryFont();
		void LoadFont(const FontSpecification& spec, bool default = false);

		const std::string& GetDefaultFontName() const { return m_DefaultFont; }
		ImFont* GetDefaultFont() { return GetFont(m_DefaultFont); }
		ImFont* GetFont(const std::string& fontName);

	private:
		std::unordered_map<std::string, ImFont*> m_Fonts;
		std::string m_DefaultFont;
		bool m_PushedTemporaryFont = false;

	};

}