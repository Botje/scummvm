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

#include "kq8/objects/object_factory.h"
#include "kq8/kq8.h"
#include "kq8/objects/connor.h"
#include "kq8/objects/interior.h"
#include "kq8/objects/monster.h"
#include "kq8/objects/object.h"
#include "kq8/objects/terrain.h"

namespace Kq8 {

ObjectFactory::ObjectFactory() {
#define FACTORY(klass) _factories["KQ" #klass] = &klass::factory
	FACTORY(Terrain);
	FACTORY(Object);
	FACTORY(Interior);
	FACTORY(Connor);
	FACTORY(Monster);
#undef FACTORY
	_factories["KQConner"] = &Connor::factory;
}

Object *ObjectFactory::load(const Common::String &klass, const KQFile &ini) {
	auto it = _factories.find(klass);
	if (it == _factories.end()) {
		return nullptr;
	}
	auto *obj = (it->_value)(ini);
	postLoad(klass, obj);
	return obj;
}

void ObjectFactory::postLoad(const Common::String &klass, Object *object) {
	if (object->addToWorld())
		g_engine->world()->addObject(object);

	if (klass == "KQTerrain") {
		auto *terrain = static_cast<Terrain *>(object);
		auto palette = g_engine->getVariable("KQWorld::terrainPalette");
		g_engine->world()->setTerrain(terrain);
		g_engine->graphicsManager().loadTerrain(terrain);
	}
}

} // namespace Kq8