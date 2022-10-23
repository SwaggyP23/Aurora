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
		void PushTemporaryFont(const std::string& fontName);
		void PopTemporaryFont();
		void LoadFont(const FontSpecification& spec, bool default = false);

		ImFont* GetFont(const std::string& fontName);

	private:
		std::unordered_map<std::string, ImFont*> m_Fonts;
		bool m_PushedTemporaryFont = false;

	};

}