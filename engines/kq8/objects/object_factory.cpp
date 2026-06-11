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
#include "kq8/objects/anim_object.h"
#include "kq8/objects/chest.h"
#include "kq8/objects/connor.h"
#include "kq8/objects/door.h"
#include "kq8/objects/interior.h"
#include "kq8/objects/monster.h"
#include "kq8/objects/object.h"
#include "kq8/objects/terrain.h"
#include "kq8/objects/trap.h"
#include "kq8/singletons/bounding_box_manager.h"
#include "kq8/singletons/inventory_item_type_list.h"
#include "kq8/singletons/monster_manager.h"
#include "kq8/singletons/monster_type_list.h"

namespace Kq8 {

ObjectFactory::ObjectFactory() {
#define FACTORY(klass) _factories["KQ" #klass] = &klass::factory
	FACTORY(Terrain);
	FACTORY(Object);
	FACTORY(Interior);
	FACTORY(Connor);
	FACTORY(Monster);
	FACTORY(AnimObject);
	FACTORY(Door);
	FACTORY(Chest);
	FACTORY(Trap);
#undef FACTORY
#define SINGLETON(klass) _factories["KQ" #klass] = &Singleton::load##klass
	SINGLETON(InventoryItemTypeList);
	SINGLETON(MonsterTypeList);
	SINGLETON(MonsterManager);
	SINGLETON(BoundingBoxManager);
#undef SINGLETON
	_factories["KQConner"] = &Connor::factory;
}

Object *ObjectFactory::load(const Common::String &klass, const KQFile &ini, bool &ok) {
	auto it = _factories.find(klass);
	if (it == _factories.end()) {
		ok = false;
		return nullptr;
	}
	ok = true;
	auto *obj = (it->_value)(ini);
	if (obj)
		postLoad(klass, obj);
	return obj;
}

Object *ObjectFactory::load(const KQFile &ini, bool &ok) {
	auto &section = ini.getSections().front();
	auto klass = section.getKey("classType")->value;
	return load(klass, ini, ok);
}

Object *ObjectFactory::restoreFromSave(CBOR::ReadStream &in) {
	in.expect(CBOR::Token::Map);
	auto kqFileK = in.readString();
	assert(kqFileK == "kqFile");
	auto kqFile = in.readString();
	auto classTypeK = in.readString();
	assert(classTypeK == "classType");
	auto classType = in.readString();

	auto nameK = in.readString();
	assert(nameK == "name");
	auto name = in.readString();

	if (kqFile.empty()) {
		error("Cannot load objects without kq file");
	}
	KQFile f;
	bool ok = f.loadFromFile(kqFile);
	if (!ok) {
		error("KQ File %s not found", kqFile.c_str());
	}

	ok = false;
	auto *obj = load(kqFile, f, ok);
	obj->loadAttributesFromStream(in);
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