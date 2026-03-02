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

#ifndef KQ8_OBJECTS_PORTAL_H
#define KQ8_OBJECTS_PORTAL_H

#include "kq8/objects/anim_object.h"

namespace Kq8 {

class Portal : public AnimObject {
public:
	static Object *factory(const KQFile &f);
	Portal(const KQFile &f);

private:
	Common::String _exitScript;
	Common::String _world;
	Common::String _enterAnim;
	Common::String _exitAnim;

	Math::Vector3d _destConnorLoc;
	Math::Vector3d _destConnorDir;
	Math::Vector3d _destCamLoc;
	Math::Vector3d _destCamDir;
};

} // namespace Kq8

#endif // KQ8_OBJECTS_PORTAL_H
