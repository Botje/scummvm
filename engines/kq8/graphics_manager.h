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

class GraphicsManager {
	Common::HashMap<Common::String, Graphics::Palette *> _palettes;

public:
	Graphics::Palette *getPalette(const Common::String &p);
	~GraphicsManager();
};

} // namespace Kq8

#endif // GRAPHICS_H
