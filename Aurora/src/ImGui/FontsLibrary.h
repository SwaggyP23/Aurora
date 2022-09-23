#pragma once

#include <unordered_map>
#include <vector>
#include <filesystem>

namespace std {

	template<typename T>
	void HaSh_CoMbInE(size_t& seed, const T& v)
	{
		seed ^= stdext::hash_value(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template<typename S, typename T>
	struct hash<std::pair<S, T>>
	{
		std::size_t operator()(const std::pair<S, T>& iden) const
		{
			std::size_t returnVal = 0;
			HaSh_CoMbInE<S>(returnVal, iden.first);
			HaSh_CoMbInE<T>(returnVal, iden.second);

			return returnVal;
		}
	};

}

namespace Aurora {

	struct  FontTypes
	{
		std::string Bold = "Empty";
		std::string Italic = "Empty";
		std::string Regular = "Empty";
		std::string Medium = "Empty";
		std::string Light = "Empty";
	};

	enum class FontIdentifier : uint32_t
	{
		None = 0, Bold, Italic, Regular, Medium, Light
	};

	class FontsLibrary
	{
	public:
		FontsLibrary() = default;

		void SetDefaultFont(const std::string& fontName, FontIdentifier type = FontIdentifier::Regular);
		void PushTemporaryFont(const std::string& fontName, FontIdentifier type);
		void PopTemporaryFont();
		void AddFont(const std::string& fontName, FontIdentifier type, float fontSize = 18.0f);
		void LoadFont(const std::string& fontName, const std::string& filepath);

		[[nodiscard]] FontTypes GetFont(const std::string& fontName);

		[[nodiscard]] inline const std::unordered_map<std::pair<std::string, FontIdentifier>, uint32_t>& GetFontNamesAndIdentifier() const { return m_FontIndices; }

	private:
		bool m_BoldHandled = false;
		bool m_ItalicHandled = false;
		bool m_RegularHandled = false;
		bool m_MediumHandled = false;
		bool m_LightHandled = false;

		bool m_PushedTemporaryFont = false;

		const char* m_ErrorToken = "Not Available";

		uint32_t m_Incrementor = 0;

		// A map of map provides very easy acces to the data, though it might not be the most efficient, it is good enough for the purposes of its usage!! So it is good enough.
		std::unordered_map<std::string, std::unordered_map<FontIdentifier, std::filesystem::path>> m_Directories;
		std::unordered_map<std::pair<std::string, FontIdentifier>, uint32_t> m_FontIndices;

	};

}