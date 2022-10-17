#include "Aurorapch.h"
#include "Font.h"

#include "MSDFData.h"
#include "AssetManager/AssetManager.h"
#include "Utils/UtilFunctions.h"

#include <msdf-atlas-gen.h>

namespace Aurora {

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define DEFAULT_MITER_LIMIT 1.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREADS 8

    namespace Utils {

        static std::filesystem::path GetCacheDirectory()
        {
            return "Resources/cache/fontAtlases";
        }

        static void CreateCacheDirectoryIfNeeded()
        {
            std::filesystem::path cacheDirectory = GetCacheDirectory();
            if (!std::filesystem::exists(cacheDirectory))
                std::filesystem::create_directories(cacheDirectory);
        }

    }

    namespace FontInternal {

        struct FontInput
        {
            const char* FontFilename;
            const char* CharsetFilename;
            const char* FontName;
            double FontScale;
            msdf_atlas::GlyphIdentifierType GlyphIdentifierType;
        };

        struct Configuration
        {
            msdf_atlas::ImageType ImageType;
            msdf_atlas::ImageFormat ImageFormat;
            msdf_atlas::YDirection YDirection;
            int Width;
            int Height;
            double EmSize;
            double PxRange;
            double AngleThreshold;
            double MiterLimiter;
            void (*EdgeColoring)(msdfgen::Shape&, double, uint64_t);
            bool ExpensiveColoring;
            uint64_t ColoringSeed;
            msdf_atlas::GeneratorAttributes GeneratorAttributes;
        };

        struct AtlasHeader
        {
            uint32_t Type = 0;
            uint32_t Width, Height;
        };

        class FontHolder
        {
        public:
            FontHolder()
                : m_FT(msdfgen::initializeFreetype()), m_Font(nullptr), m_FontFilename(nullptr) {}
            ~FontHolder()
            {
                if (m_FT)
                {
                    if (m_Font)
                        msdfgen::destroyFont(m_Font);
                    
                    msdfgen::deinitializeFreetype(m_FT);
                }
            }

            bool Load(const char* fontFilename)
            {
                if (m_FT && fontFilename)
                {
                    if (m_FontFilename && !strcmp(m_FontFilename, fontFilename))
                        return true;

                    if (m_Font)
                        msdfgen::destroyFont(m_Font);

                    m_Font = msdfgen::loadFont(m_FT, fontFilename);
                    if (m_Font)
                    {
                        m_FontFilename = fontFilename;
                        
                        return true;
                    }

                    m_FontFilename = nullptr;
                }

                return false;
            }

            operator msdfgen::FontHandle* () const
            {
                return m_Font;
            }
            
        private:
            msdfgen::FreetypeHandle* m_FT;
            msdfgen::FontHandle* m_Font;
            const char* m_FontFilename;

        };

        static bool TryReadFontAtlasFromCache(const std::string& fontName, float fontSize, AtlasHeader& header, void*& pixels, Buffer& storageBuffer)
        {
            std::string fileName = fmt::format("{0}-{1}.afa", fontName, fontSize);
            std::filesystem::path filePath = Utils::GetCacheDirectory() / fileName;

            AR_CORE_TRACE_TAG("Renderer", "Found font atlas cached at {0}", filePath);

            if (std::filesystem::exists(filePath))
            {
                storageBuffer = Utils::FileIO::ReadBytes(filePath);
                header = *storageBuffer.As<AtlasHeader>();
                pixels = (uint8_t*)storageBuffer.Data + sizeof(AtlasHeader);

                return true;
            }

            return false;
        }

        static void CacheFontAtlas(const std::string& fontName, float fontSize, const AtlasHeader& header, const void* pixels)
        {
            Utils::CreateCacheDirectoryIfNeeded();

            std::string fileName = fmt::format("{0}-{1}.afa", fontName, fontSize);
            std::filesystem::path filePath = Utils::GetCacheDirectory() / fileName;

            std::ofstream stream(filePath, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                stream.close();
                AR_CORE_ERROR_TAG("Renderer", "Failed to cache font atlas to {0}", filePath.string());
                return;
            }

            stream.write((char*)&header, sizeof(AtlasHeader));
            stream.write((char*)pixels, header.Width * header.Height * sizeof(float) * 4);
        }

        template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GEN_FN>
        static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, const Configuration& config)
        {
            msdf_atlas::ImmediateAtlasGenerator<S, N, GEN_FN, msdf_atlas::BitmapAtlasStorage<T, N>> generator(config.Width, config.Height);
            generator.setAttributes(config.GeneratorAttributes);
            generator.setThreadCount(THREADS);
            generator.generate(glyphs.data(), (int)glyphs.size());

            msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

            AtlasHeader header = {};
            header.Width = bitmap.width;
            header.Height = bitmap.height;
            CacheFontAtlas(fontName, fontSize, header, bitmap.pixels);

            TextureProperties props = {};
            props.DebugName = "FontAtlas";
            props.GenerateMips = false;
            props.SamplerWrap = TextureWrap::Clamp;
            
            return Texture2D::Create(ImageFormat::RGBA32F, header.Width, header.Height, bitmap.pixels, props);
        }

        static Ref<Texture2D> CreateCachedAtlas(const AtlasHeader& header, const void* pixels)
        {
            TextureProperties props = {};
            props.DebugName = "FontAtlas";
            props.GenerateMips = false;
            props.SamplerWrap = TextureWrap::Clamp;

            return Texture2D::Create(ImageFormat::RGBA32F, header.Width, header.Height, pixels, props);
        }

    }

    Ref<Font> Font::Create(const std::filesystem::path& filePath)
    {
        return CreateRef<Font>(filePath);
    }

    Font::Font(const std::filesystem::path& filepath)
        : m_FilePath(filepath)
    {
        m_MSDFData = new MSDFData();

        int result = 0;
        FontInternal::FontInput fontInput = {};
        fontInput.GlyphIdentifierType = msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT;
        fontInput.FontScale = -1;

        FontInternal::Configuration config = {};
        config.ImageType = msdf_atlas::ImageType::MTSDF;
        config.ImageFormat = msdf_atlas::ImageFormat::BINARY_FLOAT;
        config.YDirection = msdf_atlas::YDirection::BOTTOM_UP;
        config.EdgeColoring = msdfgen::edgeColoringInkTrap;
        config.GeneratorAttributes.config.overlapSupport = true;
        config.GeneratorAttributes.scanlinePass = true;
        config.AngleThreshold = DEFAULT_ANGLE_THRESHOLD;
        config.MiterLimiter = DEFAULT_MITER_LIMIT;
        config.EmSize = 40.0;

        const char* imageFormatName = nullptr;
        int fixedWidth = -1;
        int fixedHeight = -1;
        double minEmSize = 0.0;
        double rangeValue = 2.0;
        msdf_atlas::TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = msdf_atlas::TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;

        std::string fontFilepath = m_FilePath.string();
        fontInput.FontFilename = fontFilepath.c_str();

        // Load fonts...
        bool anyCodepointsAvailable = false;
        FontInternal::FontHolder font;

        bool success = font.Load(fontInput.FontFilename);
        AR_CORE_ASSERT(success);

        if (fontInput.FontScale <= 0.0)
            fontInput.FontScale = 1.0;

        // Load character set...
        fontInput.GlyphIdentifierType = msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT;
        msdf_atlas::Charset charSet;

        // From ImGui...
        static uint32_t charsetRange[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x2DE0, 0x2DFF, // Cyrillic Extended-A
            0xA640, 0xA69F, // Cyrillic Extended-B
            0
        };

        for (int range = 0; range < 8; range += 2)
        {
            for (uint32_t c = charsetRange[range]; c <= charsetRange[range + 1]; c++)
                charSet.add(c);
        }

        // Load glyphs...
        m_MSDFData->FontGeometry = msdf_atlas::FontGeometry(&m_MSDFData->Glyphs);
        int glyphsLoaded = -1;
        switch (fontInput.GlyphIdentifierType)
        {
            case msdf_atlas::GlyphIdentifierType::GLYPH_INDEX:
                glyphsLoaded = m_MSDFData->FontGeometry.loadGlyphset(font, fontInput.FontScale, charSet);
                break;
            case msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT:
                glyphsLoaded = m_MSDFData->FontGeometry.loadCharset(font, fontInput.FontScale, charSet);
                anyCodepointsAvailable |= glyphsLoaded > 0;
                break;
        }

        AR_CORE_ASSERT(glyphsLoaded >= 0);
        AR_CORE_TRACE_TAG("Renderer", "Loaded geometry of {0} out of {1} glyphs", glyphsLoaded, (int)charSet.size());

        // List missing glyphs...
        if (glyphsLoaded < (int)charSet.size())
        {
            AR_CORE_WARN_TAG("Renderer", "Missing {0} {1}", (int)charSet.size() - glyphsLoaded, fontInput.GlyphIdentifierType == msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT ? "codepoints" : "glyphs");
        }

        if (fontInput.FontName)
            m_MSDFData->FontGeometry.setName(fontInput.FontName);

        // Determine final atlas dimensions, scale and range, pack glyphs...
        double pxRange = rangeValue;
        bool fixedDimensions = fixedWidth >= 0 && fixedHeight >= 0;
        bool fixedScale = config.EmSize > 0;
        msdf_atlas::TightAtlasPacker atlasPacker;
        if (fixedDimensions)
            atlasPacker.setDimensions(fixedWidth, fixedHeight);
        else
            atlasPacker.setDimensionsConstraint(atlasSizeConstraint);

        // TODO: In this case (if padding == -1), the border pixels of each glyph are black, but stil computed. For floating-point
        // output, this may play a role
        atlasPacker.setPadding(config.ImageType == msdf_atlas::ImageType::MSDF || config.ImageType == msdf_atlas::ImageType::MTSDF ? 0 : -1);
    
        if (fixedScale)
            atlasPacker.setScale(config.EmSize);
        else
            atlasPacker.setMinimumScale(minEmSize);

        atlasPacker.setPixelRange(pxRange);
        atlasPacker.setMiterLimit(config.MiterLimiter);

        int remaining = atlasPacker.pack(m_MSDFData->Glyphs.data(), (int)m_MSDFData->Glyphs.size());
        if (remaining)
        {
            if (remaining < 0)
            {
                AR_CORE_ASSERT(false);
            }
            else
            {
                AR_CORE_ERROR_TAG("Renderer", "Could not fit {0} out of {1} glyphs into the atlas.", remaining, (int)m_MSDFData->Glyphs.size());
                AR_CORE_ASSERT(false);
            }
        }

        atlasPacker.getDimensions(config.Width, config.Height);
        AR_CORE_ASSERT(config.Width > 0 && config.Height > 0);

        config.EmSize = atlasPacker.getScale();
        config.PxRange = atlasPacker.getPixelRange();
        if (!fixedScale)
            AR_CORE_TRACE_TAG("Renderer", "Glyph size: {0} pixels/EM", config.EmSize);
        else
            AR_CORE_TRACE_TAG("Renderer", "Atlas dimensions: {0} x {1}", config.Width, config.Height);

        // Edge coloring...
        if (config.ImageType == msdf_atlas::ImageType::MSDF || config.ImageType == msdf_atlas::ImageType::MTSDF)
        {
            if (config.ExpensiveColoring)
            {
                msdf_atlas::Workload([&glyphs = m_MSDFData->Glyphs, &config](int i, int threadNo) -> bool
                {
                    // TODO: Why is there 2: !! in the end there????
                    uint64_t glyphSeed = (LCG_MULTIPLIER * (config.ColoringSeed ^ i) + LCG_INCREMENT) * !!config.ColoringSeed;
                    glyphs[i].edgeColoring(config.EdgeColoring, config.AngleThreshold, glyphSeed);

                    return true;
                }, (int)m_MSDFData->Glyphs.size()).finish(THREADS);
            }
            else
            {
                uint64_t glyphSeed = config.ColoringSeed;
                for (msdf_atlas::GlyphGeometry& glyph : m_MSDFData->Glyphs)
                {
                    glyphSeed *= LCG_MULTIPLIER;
                    glyph.edgeColoring(config.EdgeColoring, config.AngleThreshold, glyphSeed);
                }
            }
        }

        std::string fontName = filepath.filename().string();

        // Check cache here...
        Buffer storageBuffer;
        FontInternal::AtlasHeader header;
        void* pixels;

        if (FontInternal::TryReadFontAtlasFromCache(fontName, (float)config.EmSize, header, pixels, storageBuffer))
        {
            m_TextureAtlas = FontInternal::CreateCachedAtlas(header, pixels);
            storageBuffer.Release();
        }
        else
        {
            AR_CORE_TRACE_TAG("Renderer", "Did not find any font atlas cached!");
            bool floatingPointFormat = true;
            Ref<Texture2D> texture;

            switch (config.ImageType)
            {
                case msdf_atlas::ImageType::MSDF:
                    if (floatingPointFormat)
                        texture = FontInternal::CreateAndCacheAtlas<float, float, 3, msdf_atlas::msdfGenerator>(fontName, (float)config.EmSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
                    else
                        texture = FontInternal::CreateAndCacheAtlas<Byte, float, 3, msdf_atlas::msdfGenerator>(fontName, (float)config.EmSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
                    break;
                case msdf_atlas::ImageType::MTSDF:
                    if(floatingPointFormat)
                         texture = FontInternal::CreateAndCacheAtlas<float, float, 4, msdf_atlas::mtsdfGenerator>(fontName, (float)config.EmSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
                    else
                        texture = FontInternal::CreateAndCacheAtlas<Byte, float, 4, msdf_atlas::mtsdfGenerator>(fontName, (float)config.EmSize, m_MSDFData->Glyphs, m_MSDFData->FontGeometry, config);
                    break;
            }

            m_TextureAtlas = texture;
        }
    }

    Font::~Font()
    {
        delete m_MSDFData;
    }

    void Font::Init()
    {
        s_DefaultFont = Font::Create("Resources/EditorInternal/Fonts/OpenSans/OpenSans-Bold.ttf");
    }

    void Font::Shutdown()
    {
        s_DefaultFont.Reset();
    }

    Ref<Font> Font::GetDefaultFont()
    {
        return s_DefaultFont;
    }

    Ref<Font> Font::GetFontAssetForTextComponent(const TextComponent& textComponent)
    {
        if (textComponent.FontHandle == s_DefaultFont->Handle || !AssetManager::IsAssetHandleValid(textComponent.FontHandle))
        {
            return s_DefaultFont;
        }

        return AssetManager::GetAsset<Font>(textComponent.FontHandle);
    }

    Ref<Font> Font::s_DefaultFont;

}