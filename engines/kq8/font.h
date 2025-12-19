/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KQ8_FONT_H
#define KQ8_FONT_H

#include "common/ptr.h"
#include "graphics/surface.h"

#include "kq8/graphics_manager.h"

namespace Kq8 {

class Font {
	using SurfacePtr = Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter>;
	using CharMap = Common::HashMap<unsigned char, unsigned short>;

	friend class GraphicsManager;

	uint32 _numGlyphs;
	CharMap _charToGlyph;
	SurfacePtr _atlas;
	Common::Rect _boundingBox;

	Font(uint32 numGlyphs, const CharMap &charToGlyph, Graphics::Surface *atlas)
		: _numGlyphs(numGlyphs), _charToGlyph(charToGlyph), _atlas(atlas) {}

public:
	static Font *loadFont(const Common::String &path, const Graphics::Palette *palette);
	Font() = default;
	uint32 numGlyphs() const { return _numGlyphs; }
	const CharMap &charToGlyph() const { return _charToGlyph; }
	const Graphics::Surface *atlas() const { return _atlas.get(); }
	const Common::Rect &boundingBox() const { return _boundingBox; }
};

} // namespace Kq8

#endif // KQ8_FONT_H
