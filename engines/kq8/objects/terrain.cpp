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

#include "kq8/kq8.h"
#include "kq8/objects/terrain.h"

#include "kq8/graphics_manager.h"

namespace Kq8 {

Object *Terrain::factory(const KQFile &f) {
	return new Terrain(f);
}
Terrain::Terrain(const KQFile &f)
	: Object(f) {
	auto &section = f.getSections().front();
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

	auto paletteName = g_engine->getVariable("KQWorld::terrainPalette");
	auto palette = g_engine->graphicsManager().getPalette(paletteName);

	auto terrainFlags = loadTerrainFlags();

	_width = heights->w;
	_height = heights->h;
	_tiles.resize(_width * _height);
	for (auto &materialName : materialsAndMapping.first) {
		_materials.emplace_back(g_engine->graphicsManager().loadBitmap(materialName, palette, GraphicsManager::BitmapPacking::kLoose));
	}

	for (int r = 0; r < heights->h; r++) {
		for (int c = 0; c < heights->w; c++) {
			uint8 height = heights->getPixel(c, heights->h - r - 1);
			// heights is 129x129, materials is only 128x128. Clamp any overflows to the border.
			uint8 mat = materials->getPixel(MIN<int>(c, materials->w - 1), MAX<int>(int(materials->h) - r - 1, 0));
			tileAt(c, r) = Tile{height, materialsAndMapping.second[mat], terrainFlags[mat]};
		}
	}
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

Common::Pair<Common::Array<Common::String>, Common::Array<uint8> > Terrain::loadMaterialFile(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{name}));
	stream->skip(34);
	auto num_materials = stream->readUint32LE();

	Common::Pair<Common::Array<Common::String>, Common::Array<uint8> > ret;
	Common::Array<Common::String> &materials = ret.first;
	Common::Array<uint8> &mapping = ret.second;

	while (num_materials--) {
		/* auto flags = */ stream->readUint16LE();
		stream->skip(14);
		char mat[16];
		stream->read(mat, sizeof(mat));

		auto materialIt = Common::find(materials.begin(), materials.end(), mat);
		if (materialIt != materials.end()) {
			mapping.push_back(Common::distance(materials.begin(), materialIt));
		} else {
			materials.push_back(mat);
			mapping.push_back(materials.size() - 1);
		}
	}
	return ret;
}

} // namespace Kq8