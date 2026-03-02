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

#include "kq8/objects/portal.h"

namespace Kq8 {

using namespace INIHelpers;

Object *Portal::factory(const KQFile &f) {
	return new Portal(f);
}

Portal::Portal(const KQFile &f) : AnimObject{f} {
	const auto &section = f.getSections().front();

	_exitScript = get<Common::String>(section, "exitScript");
	_world = get<Common::String>(section, "world");
	_enterAnim = get<Common::String>(section, "enterAnim");
	_exitAnim = get<Common::String>(section, "exitAnim");

	_destConnorLoc = get(section, "destConnorLocX", "destConnorLocY", "destConnorLocZ");
	_destConnorDir = get(section, "destConnorDirX", "destConnorDirY", "destConnorDirZ");
	_destCamLoc = get(section, "destCamLocX", "destCamLocY", "destCamLocZ");
	_destCamDir = get(section, "destCamDirX", "destCamDirY", "destCamDirZ");
}

} // namespace Kq8