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

#ifndef KQ8_OBJECTS_TERRAIN_H
#define KQ8_OBJECTS_TERRAIN_H

#include "common/util.h"

#include "kq8/objects/object.h"

namespace Kq8 {

class Bitmap;

class Terrain : public Object {
public:
	enum TerrainFlag : uint16 {
		empty = 0,
		inUse = 1 << 0,
		dirt = 1 << 1,
		grass = 1 << 2,
		rock = 1 << 3,
		water = 1 << 4,
		unused1 = 1 << 5,
		weird2 = 1 << 6,
		poison = 1 << 7,
		fastpoison = 1 << 8,
		swamp = 1 << 9,
		fire = 1 << 10,
		ice = 1 << 11,
		ontop = 1 << 12,
	};
	struct Tile {
		uint8 height;
		uint8 material;
		TerrainFlag flags;
	};

	static Object *factory(const KQFile &f);
	Terrain(const KQFile &f);

	inline Tile &tileAt(uint8 x, uint8 y) {
		return _tiles[y * _width + x];
	}

	const Common::Array<Tile> &tiles() const { return _tiles; }
	const Common::Array<Kq8::Bitmap *> &materials() const { return _materials; }
	uint8 width() const { return _width - 1; }
	uint8 height() const { return _height - 1; }

private:
	Common::Array<Tile> _tiles;
	Common::Array<Kq8::Bitmap *> _materials;
	uint8 _width;
	uint8 _height;
	Common::Array<TerrainFlag> loadTerrainFlags();
	Common::Pair<Common::Array<Common::String>, Common::Array<uint8> > loadMaterialFile(const Common::String &string);
};

} // namespace Kq8

#endif // TERRAIN_H
