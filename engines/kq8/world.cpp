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

#include "kq8/world.h"

#include "kq8/kq8.h"
#include "kq8/objects/camera.h"
#include "kq8/objects/terrain.h"

namespace Kq8 {

World::World(const Common::String &name)
	: _name(name) {
	addObject(new Camera());
}

World::~World() {
	delete _terrain;
}

const Graphics::Palette *World::getObjectPalette() {
	auto paletteName = g_engine->getVariable("KQWorld::objectPalette");
	return g_engine->graphicsManager().getPalette(paletteName);
}

void World::addObject(Object *object) {
	_objects.push_back(object);
}

void World::update(float dt) {
	for (auto *object : _objects) {
		object->update(dt);
	}

	if (!_toDelete.empty()) {
		auto it = _objects.begin();
		while (it != _objects.end() && !_toDelete.empty()) {
			auto toDeleteIt = Common::find(_toDelete.begin(), _toDelete.end(), *it);
			if (toDeleteIt != _toDelete.end()) {
				*it = _objects.back();
				_objects.pop_back();
				_toDelete.erase(toDeleteIt);
			} else {
				++it;
			}
		}
		_toDelete.clear();
	}
}

void World::draw() {
	if (_terrain) {
		g_engine->gfx().drawTerrain(_terrain);
	}

	auto visibleDistance = _terrain->visibleDistance();
	auto camPos = camera()->pos();

	for (auto *object : _objects) {
		// auto toObject = object->pos() - camPos;
		// auto dot = camera()->direction().dotProduct(toObject);
		// if (dot < 0 || dot > visibleDistance * visibleDistance) {
		// 	continue;
		// }
		object->draw();
	}
}

Object *World::findObject(const Common::String &name) {
	for (auto *object : _objects) {
		if (object->name() == name) {
			return object;
		}
	}
	return nullptr;
}
void World::deleteLater(Object *obj) {
	_toDelete.push_back(obj);
}
Camera *World::camera() {
	return (Camera *)findObject("KQCamera");
}
Connor *World::connor() {
	return (Connor *)findObject("Connor");
}

} // namespace Kq8