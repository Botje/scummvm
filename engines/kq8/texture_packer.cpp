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

#include "texture_packer.h"

#include "graphics/opengl/debug.h"
#include "math/vector2d.h"

#if defined(USE_OPENGL) || defined(USE_OPENGL_SHADERS)

namespace Kq8 {

static bool fits(const Common::Rect &big, const Common::Rect &small) {
	return big.width() >= small.width() && big.height() >= small.height();
}

void SubTexture::copyFrom(Bitmap *bmp) {
	if (!texture->bind()) {
		return;
	}
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
							rect.left, rect.top,
							rect.width(), rect.height(),
							GL_RGB, GL_UNSIGNED_BYTE,
							bmp->surface()->getPixels()));
}

const Math::Vector2d SubTexture::getTexOffset() const {
	return Math::Vector2d{static_cast<float>(rect.left) + 0.5f, static_cast<float>(rect.top) + 0.5f} / Math::Vector2d{static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight())};
}
const Math::Vector2d SubTexture::getTexSize() const {
	return Math::Vector2d{static_cast<float>(rect.width() - 1), static_cast<float>(rect.height() - 1)} / Math::Vector2d{static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight())};
}

bool TexturePacker::assign(const Common::Rect &piece, Common::Rect &location) {
	if (!currentRow.isEmpty() && fits(currentRow, piece)) {
		location = Common::Rect{currentRow.origin(), piece.width(), piece.height()};
		currentRow.translate(piece.width(), 0);
		currentRow.setWidth(currentRow.width() - piece.width());
		return true;
	} else if (fits(remainder, piece)) {
		location = Common::Rect{remainder.origin(), piece.width(), piece.height()};
		currentRow = Common::Rect{
			remainder.origin() + Common::Point{piece.width(), 0},
			static_cast<int16>(remainder.width() - piece.width()),
			piece.height()};
		remainder.setHeight(remainder.height() - piece.height());
		remainder.translate(0, piece.height());
		return true;
	}
	return false;
}
} // namespace Kq8
#endif