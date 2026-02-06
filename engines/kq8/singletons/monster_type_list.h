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

#ifndef KQ8_MONSTER_TYPE_LIST_H
#define KQ8_MONSTER_TYPE_LIST_H

#include "kq8/objects/object.h"

namespace Kq8 {

struct MonsterType {
	Common::String _type;
	uint16 _health;
	uint16 _damage;
	uint16 _protection;
	uint16 _experience;
	Common::String _kqFile;
	Common::String _movement;
	Common::String _invHold;
	Common::String _invDrop;
	Common::String _chaseMode;
	uint16 _alarmRadius;
	float _birthDelay;
	float _attackDist;
	float _attackDelay;
};

namespace Singleton {
Object *loadMonsterTypeList(const KQFile &ini);
}

} // namespace Kq8

#endif // KQ8_MONSTER_TYPE_LIST_H
