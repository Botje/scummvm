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

void World::draw() {
	if (_terrain) {
		g_engine->gfx().drawTerrain(_terrain);
	}

	for (auto *object : _objects) {
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

} // namespace Kq8