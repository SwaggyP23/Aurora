#pragma once

#include <msdf-atlas-gen.h>
#include <vector>

namespace Aurora {

	struct MSDFData
	{
		msdf_atlas::FontGeometry FontGeometry;
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
	};

}