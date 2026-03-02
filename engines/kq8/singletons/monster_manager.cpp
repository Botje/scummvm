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

#include "kq8/singletons/monster_manager.h"

#include "kq8/kq8.h"
#include "kq8/objects/monster.h"
#include "kq8/script_tokenizer.h"

namespace Kq8 {
using namespace INIHelpers;

static float atOrFloat(const Common::String &token, float def) {
	if (token == "@")
		return def;
	return ScriptTokenizer{token}.nextFloat();
}
void spawnMonster(const Common::String &line) {
	ScriptTokenizer t{line};
	Common::String name = t.nextToken();
	Common::String monsterType = t.nextToken();
	const MonsterType &mt = g_engine->reference()._monsterTypes[monsterType];
	Common::String team = t.nextToken();
	uint16 health = mt._health + atOrFloat(t.nextToken(), 0);
	uint16 damage = mt._damage + atOrFloat(t.nextToken(), 0);
	uint16 protection = mt._protection + atOrFloat(t.nextToken(), 0);
	uint16 experience = mt._experience + atOrFloat(t.nextToken(), 0);
	float locX = t.nextFloat();
	float locY = t.nextFloat();
	float locZ = t.nextFloat();
	float dirZ = t.nextFloat();
	Common::String dependents = t.nextToken();
	Common::String movement = t.nextToken();
	Common::String invHold = t.nextToken();
	Common::String invDrop = t.nextToken();
	Common::String chaseMode = t.nextToken();
	uint16 alarmRadius = mt._alarmRadius + atOrFloat(t.nextToken(), 0);
	float birthDelay = mt._birthDelay + atOrFloat(t.nextToken(), 0);
	float attackDist = mt._attackDist + atOrFloat(t.nextToken(), 0);
	float attackDelay = mt._attackDelay + atOrFloat(t.nextToken(), 0);

	KQFile ini;
	auto stream = Common::ScopedPtr<Common::SeekableReadStream>{SearchMan.createReadStreamForMember(Common::Path{mt._kqFile})};

	if (!stream) {
		warning("Could not loadKQ '%s'", mt._kqFile.c_str());
		return;
	}
	ini.loadFromStream(*stream);
	bool ok = false;
	Monster *monster = static_cast<Monster *>(g_engine->objectFactory().load(ini, ok));
	if (!ok) {
		warning("Could not instantiate monster '%s'", name.c_str());
		return;
	}

	monster->setName(name);
	monster->moveTo(Math::Vector3d{locX, locY, locZ});
	monster->setRotation(Math::Vector3d{0, 0, dirZ});
	monster->setAlarmRadius(alarmRadius);
}

Object *
Singleton::loadMonsterManager(const KQFile &ini) {
	auto lines = getArray<Common::String>(ini.getSections().front(), "numMonsters", "monster");
	for (const auto &line : lines) {
		spawnMonster(line);
	}
	return nullptr;
}
} // namespace Kq8