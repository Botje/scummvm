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

#ifndef PIXEL_FORMATS_H
#define PIXEL_FORMATS_H

#include "common/scummsys.h"
#include "graphics/pixelformat.h"

namespace Kq8 {

struct PixelFormats {
	static inline const Graphics::PixelFormat getRGBPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
#else
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
#endif
	}
};

} // namespace Kq8

#endif // PIXEL_FORMATS_H
