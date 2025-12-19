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

#ifndef KQ8_BITMAP_H
#define KQ8_BITMAP_H

#include "common/ptr.h"
#include "common/str.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}
namespace Kq8 {

class Bitmap {
	friend class GraphicsManager;

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _surface;

public:
	static Bitmap *loadBitmap(const Common::String &path, const Graphics::Palette *palette);
	static Graphics::Surface *parseBitmap(Common::SeekableReadStream *stream);

	Bitmap() = default;
	explicit Bitmap(Graphics::Surface *surface) : _surface{surface} {}

	Graphics::Surface *surface() const { return _surface.get(); }
};

} // namespace Kq8

#endif // KQ8_BITMAP_H
