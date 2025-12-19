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

#include "common/archive.h"
#include "common/stream.h"
#include "graphics/surface.h"

#include "kq8/bitmap.h"
#include "kq8/font.h"
#include "kq8/pixel_formats.h"

namespace Kq8 {

Font *Font::loadFont(const Common::String &path, const Graphics::Palette *palette) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path(path)));
	if (!stream) {
		warning("Could not load font '%s'", path.c_str());
		return nullptr;
	}

	auto tag = stream->readUint32BE();
	if (tag != MKTAG('P', 'F', 'O', 'N')) {
		warning("wrong tag for font, expected PFON");
		return nullptr;
	}
	stream->skip(16);
	auto numGlyphs = stream->readUint32LE();
	auto maxHeight = stream->readUint32LE();
	auto maxWidth = stream->readUint32LE();
	stream->skip(24);

	uint32 alphabetSize = stream->readUint32LE();
	Common::HashMap<uint8, uint16> charToGlyph;
	for (uint32 i = 0; i < alphabetSize; i++) {
		int16 c = stream->readSint16LE();
		if (c != -1) {
			charToGlyph[c] = i;
		}
	}

	stream->skip(numGlyphs * 2 * 4);

	tag = stream->readUint32BE();
	if (tag != MKTAG('P', 'B', 'M', 'A')) {
		warning("wrong tag for font, expected PBMA");
		return nullptr;
	}
	/* auto pbmaLen = */ stream->readUint32LE();

	tag = stream->readUint32BE();
	if (tag != MKTAG('h', 'e', 'a', 'd')) {
		warning("wrong tag for font, expected head");
		return nullptr;
	}
	/* auto headLen = */ stream->readUint32LE();
	auto numRMaps = stream->readUint32LE();
	auto numBitmaps = stream->readUint32LE();

	tag = stream->readUint32BE();
	if (tag != MKTAG('r', 'm', 'a', 'p')) {
		warning("wrong tag for font, expected head");
		return nullptr;
	}
	// rmap lists the characters in order
	uint32 rmapLen = stream->readUint32LE() / 4;
	Common::Array<uint8> chars;
	while (rmapLen-- > 0) {
		chars.push_back(stream->readUint32LE());
	}

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> atlas;
	atlas.reset(new Graphics::Surface);
	// TODO: what if numGlyphs * maxWidth overflows an int16?
	atlas->create(numGlyphs * maxWidth, maxHeight, Graphics::PixelFormat::createFormatCLUT8());

	for (int i = 0; i < numBitmaps; i++) {
		auto *surface = Bitmap::parseBitmap(stream.get());
		if (!surface) {
			warning("Could not parse bitmap %d", i);
			return nullptr;
		}
		atlas->copyRectToSurface(*surface, i * maxWidth, 0, Common::Rect{surface->w, surface->h});
	}

	atlas->convertToInPlace(PixelFormats::getRGBPixelFormat(), palette->data(), palette->size());

	return new Font{numGlyphs, charToGlyph, atlas.release()};
}
} // namespace Kq8