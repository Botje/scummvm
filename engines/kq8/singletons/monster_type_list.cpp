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

#include "kq8/singletons/monster_type_list.h"

#include "kq8/kq8.h"
#include "kq8/script_tokenizer.h"

namespace Kq8 {
using namespace INIHelpers;

template<>
inline MonsterType INIHelpers::extractFromValue<MonsterType>(const Common::String &line) {
	ScriptTokenizer t{line};
	MonsterType ret;

	/* class = */ t.nextToken();
	ret._type = t.nextToken();
	ret._health = t.nextUint16();
	ret._damage = t.nextUint16();
	ret._protection = t.nextUint16();
	ret._experience = t.nextUint16();
	ret._kqFile = t.nextToken();
	ret._movement = t.nextToken();
	ret._invHold = t.nextToken();
	ret._invDrop = t.nextToken();
	ret._chaseMode = t.nextToken();
	ret._alarmRadius = t.nextUint16();
	ret._birthDelay = t.nextFloat();
	ret._attackDist = t.nextFloat();
	ret._attackDelay = t.nextFloat();

	return ret;
}

Object *Singleton::loadMonsterTypeList(const KQFile &ini) {
	auto monsters = getArray<MonsterType>(ini.getSections().front(), "numTypes", "type");
	auto &monsterTypes = g_engine->reference()._monsterTypes;
	for (const auto &monsterType : monsters) {
		monsterTypes.setVal(monsterType._type, monsterType);
	}
	return nullptr;
}
} // namespace Kq8