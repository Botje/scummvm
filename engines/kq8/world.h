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

#ifndef KQ8_WORLD_H
#define KQ8_WORLD_H

#include "common/ptr.h"
#include "common/str.h"
#include "objects/object.h"
#include "objects/world_item.h"

namespace Graphics {
class Palette;
}

namespace Kq8 {

class Camera;
class Connor;
class Terrain;

class World {
public:
	World(const Common::String &name);
	~World();

	Terrain *terrain() { return _terrain; }
	void setTerrain(Terrain *terrain) { _terrain = terrain; }
	const Graphics::Palette *getObjectPalette();
	void addObject(Object *object);
	void update(float dt);
	void draw();
	Object *findObject(const Common::String &name);
	void deleteLater(Object *obj);
	Camera *camera();
	Connor *connor();

private:
	Common::String _name;
	Terrain *_terrain = nullptr;
	Common::Array<Object *> _objects;
	Common::Array<Object *> _toDelete;
};

} // namespace Kq8

#endif // WORLD_H
