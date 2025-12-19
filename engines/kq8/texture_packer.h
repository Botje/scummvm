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

#ifndef KQ8_TEXTURE_PACKER_H
#define KQ8_TEXTURE_PACKER_H
#include "math/vector2d.h"

#if defined(USE_OPENGL) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/texture.h"
#include "math/mathfwd.h"

#include "kq8/bitmap.h"

namespace Kq8 {

struct SubTexture {
	OpenGL::Texture *texture;
	Common::Rect rect;
	SubTexture() = default;
	SubTexture(OpenGL::Texture *texture, const Common::Rect &rect)
		: texture{texture}, rect{rect} {}
	SubTexture(OpenGL::Texture *texture, const Common::Rect &rect, Bitmap *bmp)
		: SubTexture{texture, rect} {
		copyFrom(bmp);
	}
	const Math::Vector2d getTexOffset() const;
	const Math::Vector2d getTexSize() const;
	void copyFrom(Bitmap *bmp);
};

struct TexturePacker {
	explicit TexturePacker(OpenGL::Texture *texture)
		: texture{texture}, currentRow{0, 0},
		  remainder{static_cast<int16>(texture->getWidth()), static_cast<int16>(texture->getHeight())} {}
	OpenGL::Texture *texture;
	Common::Rect currentRow;
	Common::Rect remainder;
	bool assign(const Common::Rect &piece, Common::Rect &location);
};

} // namespace Kq8

#endif

#endif // KQ8_TEXTURE_PACKER_H
