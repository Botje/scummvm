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

#include "common/archive.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "image/bmp.h"

#include "kq8/graphics_manager.h"
#include "kq8/kq8.h"
#include "kq8/material_file.h"
#include "kq8/objects/terrain.h"

namespace Kq8 {

Object *Terrain::factory(const KQFile &f) {
	return new Terrain(f);
}

Common::Array<uint8> loadMaterialDirectionFile(const Common::String &path) {
	Common::Array<uint8> directions;
	directions.resize(256, 0);
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	// hack: daventry's terrain.kq specifies terrain/daventry.bin
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{path}.getLastComponent()));
	if (!stream) {
		warning("Cannot load material direction file '%s'", path.c_str());
		return directions;
	}

	auto length = stream->readUint32LE();
	for (int i = 0; i < length; i++) {
		/* auto seq = */ stream->readUint32LE();
		stream->skip(16 + 4);
		directions[i] = stream->readUint32LE();
	}

	return directions;
}

// Terrain in KQ8 is represented as a combination of:
// - `heightBMP`: a grayscale image, with values scaled by `heightBMPScale`
// - `materialBMP`: a grayscale image with values referring to `materialListFile`
// - `floorBMPName`: ?
// - `materialDirection`: ?
// - `groundScale`: log(2) of the size of one pixel in the heightmap
// Locations in the game are given with the Y axis pointing up, so (0,0)
// is the bottom left corner.
Terrain::Terrain(const KQFile &f)
	: Object(f) {
	auto &section = f.getSections().front();
	_groundScale = 1 << strtol(section.getKey("groundScale")->value.c_str(), nullptr, 10);
	_heightScale = strtol(section.getKey("heightBMPScale")->value.c_str(), nullptr, 10);
	_visibleDistance = strtol(section.getKey("visibleDistance")->value.c_str(), nullptr, 10);

	Image::BitmapDecoder heightDecoder;
	{
		auto heightBMP = section.getKey("heightBMP")->value;
		Common::ScopedPtr<Common::SeekableReadStream> stream{SearchMan.createReadStreamForMember(Common::Path{
			heightBMP})};
		heightDecoder.loadStream(*stream);
	}

	Image::BitmapDecoder materialDecoder;
	{
		auto materialBMP = section.getKey("materialBMP")->value;
		Common::ScopedPtr<Common::SeekableReadStream> stream{SearchMan.createReadStreamForMember(Common::Path{
			materialBMP})};
		materialDecoder.loadStream(*stream);
	}
	auto heights = heightDecoder.getSurface();
	auto materials = materialDecoder.getSurface();

	auto materialListFile = section.getKey("materialListFile")->value;
	auto materialsAndMapping = loadMaterialFile(materialListFile);
	auto materialDirectionFile = section.getKey("materialDirection")->value;
	auto materialDirection = loadMaterialDirectionFile(materialDirectionFile);

	auto paletteName = g_engine->getVariable("KQWorld::terrainPalette");
	auto palette = g_engine->graphicsManager().getPalette(paletteName);

	auto terrainFlags = loadTerrainFlags();

	_width = materials->w;
	_height = materials->h;
	_tiles.resize(_width * _height);
	for (auto &materialName : materialsAndMapping.first) {
		_materials.emplace_back(g_engine->graphicsManager().loadBitmap(materialName, palette, GraphicsManager::BitmapPacking::kLoose));
	}

	// The heightmap describes height values at the corners of a tile, the materialmap determines texture and properties.
	for (int r = 0; r < _height; r++) {
		for (int c = 0; c < _width; c++) {
			uint8 heightNW = heights->getPixel(c + 0, heights->h - 1 - (r + 0));
			uint8 heightNE = heights->getPixel(c + 1, heights->h - 1 - (r + 0));
			uint8 heightSW = heights->getPixel(c + 0, heights->h - 1 - (r + 1));
			uint8 heightSE = heights->getPixel(c + 1, heights->h - 1 - (r + 1));
			uint8 mat = materials->getPixel(c, materials->h - 1 - r);
			tileAt(c, r) = Tile{{heightNW, heightNE, heightSW, heightSE}, materialsAndMapping.second[mat], materialDirection[mat], terrainFlags[mat]};
		}
	}
}

static Math::Vector3d barycentricCoordinates(const Math::Vector2d &point, const Common::Array<Math::Vector3d> &vs) {
	float area = 1.0f / 2.0f * (-vs[1].y() * vs[2].x() + vs[0].y() * (-vs[1].x() + vs[2].x()) + vs[0].x() * (vs[1].y() - vs[2].y()) + vs[1].x() * vs[2].y());

	float a1 = (vs[0].y() * vs[2].x() - vs[0].x() * vs[2].y() + (vs[2].y() - vs[0].y()) * point.getX() + (vs[0].x() - vs[2].x()) * point.getY()) / (2.0f * area);
	float a2 = (vs[0].x() * vs[1].y() - vs[0].y() * vs[1].x() + (vs[0].y() - vs[1].y()) * point.getX() + (vs[1].x() - vs[0].x()) * point.getY()) / (2.0f * area);

	return Math::Vector3d{1.0f - a1 - a2, a1, a2};
}

Math::Vector2d Terrain::worldPosToTile(float &x, float &y) const {
	x /= groundScale();
	y /= groundScale();
	Math::Vector2d remainder{fmod(x, 1.0f), fmod(y, 1.0f)};
	return remainder;
}

float Terrain::adaptZ(float x, float y) const {
	Math::Vector2d remainder = worldPosToTile(x, y);
	const auto &tile = tileAt(x, y);
	Common::Array<Math::Vector3d> triangle;
	using Corner = Tile::Corner;
	if (remainder.getY() < remainder.getX()) {
		triangle = {{0, 0, float(tile.heights[Corner::NW])}, {1, 1, float(tile.heights[Corner::SE])}, {1, 0, float(tile.heights[Corner::NE])}};
	} else {
		triangle = {{0, 0, float(tile.heights[Corner::NW])}, {1, 1, float(tile.heights[Corner::SE])}, {0, 1, float(tile.heights[Corner::SW])}};
	}
	auto weights = barycentricCoordinates(remainder, triangle);
	return (weights.x() * triangle[0].z() + weights.y() * triangle[1].z() + weights.z() * triangle[2].z()) * heightScale();
}

Common::Array<Terrain::TerrainFlag> Terrain::loadTerrainFlags() {
	Common::Array<Terrain::TerrainFlag> ret;
	KQFile f;
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{"envInfo.kq"}));
	f.loadFromStream(*stream);
	auto &section = f.getSections().front();
	for (const auto &kv : section.getKeys()) {
		if (kv.key.hasPrefix("texture")) {
			uint16 flag = 0;
			uint16 cur = 1;
			for (auto c : kv.value) {
				if (c == 'x') {
					flag |= cur;
				} else if (c == '\t') {
					cur <<= 1;
				}
			}
			ret.push_back(static_cast<TerrainFlag>(flag));
		}
	}
	return ret;
}

} // namespace Kq8