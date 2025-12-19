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

#include "kq8/objects/connor.h"

#include "kq8/kq8.h"

namespace Kq8 {

Object *Connor::factory(const KQFile &f) {
	return new Connor(f);
}
Connor::Connor(const KQFile &f) : Object{f, false} {
}

void Connor::startSpecialAnimation(const Common::String &animListName, const Common::Array<Common::String> &loops) {
	auto *loopList = g_engine->graphicsManager().loadAnimationLoopList(animListName);
	_specialAnimation.reset(new SpecialAnimation{loopList, loops});
}

} // namespace Kq8