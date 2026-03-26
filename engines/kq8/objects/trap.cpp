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

#include "kq8/objects/trap.h"

#include "kq8/kq8.h"

namespace Kq8 {
using namespace INIHelpers;

Object *Trap::factory(const KQFile &f) {
	return new Trap(f);
}

Trap::Trap(const KQFile &f) : Object(f) {
	auto &section = f.getSections().front();
	_boundingBox._min = get(section, "BBMinX", "BBMinY", "BBMinZ");
	_boundingBox._max = get(section, "BBMaxX", "BBMaxY", "BBMaxZ");
	_colliderMask = get<uint16>(section, "colliderMask");
}

bool Trap::collide(Object *collider, const Math::Vector3d &newPos) {
	if (collider->colliderMask() & this->colliderMask()) {
		Script::Args args{_name, "On"};
		g_engine->runScript(_script, args);
	}
	return false;
}

} // namespace Kq8