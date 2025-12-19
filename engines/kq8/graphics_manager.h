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

#ifndef KQ8_GRAPHICSMANAGER_H
#define KQ8_GRAPHICSMANAGER_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "graphics/palette.h"

namespace Kq8 {

class Font;
class Bitmap;

class GraphicsManager {

public:
	Common::HashMap<Common::String, Graphics::Palette *> _palettes;
	Common::HashMap<Common::String, Kq8::Font *> _fonts;
	Common::HashMap<Common::String, Kq8::Bitmap *> _bitmaps;
	uint32 numFonts;

public:
	Graphics::Palette *getPalette(const Common::String &p);
	~GraphicsManager();

	template<class Base>
	struct ManagedResource {
		uint32 _handle;
		ManagedResource() : _handle(0) {}
		explicit ManagedResource(uint32 handle)
			: _handle{handle} {}
		ManagedResource(const ManagedResource &other)
			: _handle(other._handle) {}
		ManagedResource(ManagedResource &&other) noexcept
			: _handle{0} {
			SWAP(_handle, other._handle);
		}
		ManagedResource &operator=(const ManagedResource &other) {
			if (this == &other || other._handle == _handle)
				return *this;
			_handle = other._handle;
			return *this;
		}
		ManagedResource &operator=(ManagedResource &&other) noexcept {
			if (this == &other || other._handle == _handle)
				return *this;
			_handle = other._handle;
			return *this;
		}
	};
	struct Font : public ManagedResource<Font> {
		explicit Font(uint32 handle = 0)
			: ManagedResource{handle} {}
	};

	struct Bitmap : public ManagedResource<Bitmap> {
		explicit Bitmap(uint32 handle = 0)
			: ManagedResource{handle} {}
	};

	Font loadFont(const Common::String &name, const Graphics::Palette *palette);
	Bitmap loadBitmap(const Common::String &name);
};

} // namespace Kq8

#endif // GRAPHICS_H
