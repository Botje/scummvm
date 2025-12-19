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

#ifndef KQ8_MAIN_SCREEN_H
#define KQ8_MAIN_SCREEN_H

#include "common/array.h"
#include "common/str.h"

namespace Kq8 {

class MainScreen {
	struct ScreenItem;
	uint32 _width;
	uint32 _height;
	Common::String _palette;
	Common::Array<ScreenItem> _items;

public:
	MainScreen(const Common::String &palette);
	~MainScreen();
	void prepare();
	void draw();
};

} // namespace Kq8

#endif // KQ8_MAIN_SCREEN_H
