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

#include "kq8/material_file.h"
#include "kq8/objects/interior.h"

#include "kq8/kq8.h"
#include "kq8/read_helpers.h"

namespace Kq8 {

Object *Interior::factory(const KQFile &f) {
	return new Interior(f);
}

Interior::Interior(const KQFile &f)
	: Object(f, false) {
	auto &section = f.getSections().front();
	_shapeName = section.getKey("shapeName")->value;
	const auto &materialFile = section.getKey("material")->value;

	const auto pair = loadMaterialFile(materialFile);
	const auto &materials = pair.first;
	const auto &materialMapping = pair.second;

	auto paletteName = g_engine->getVariable("KQWorld::interiorPalette");
	auto palette = g_engine->graphicsManager().getPalette(paletteName);

	for (const auto &materialName : materials) {
		_materials.push_back(g_engine->graphicsManager().loadBitmap(materialName, palette, GraphicsManager::BitmapPacking::kLoose));
	}

	loadShape(_shapeName, materialMapping);
	g_engine->graphicsManager().loadInterior(this);
}
void Interior::draw() {
	g_engine->gfx().drawInterior(this);
}

int16 Interior::evaluateBSP(const Math::Vector3d &pos) {
	int16 idx = 0;
	while (idx >= 0) {
		const auto &node = _bspNodes[idx];
		const auto &plane = _planes[node._planeIndex];
		float result = plane.dotProduct(Math::Vector4d{pos.x(), pos.y(), pos.z(), -1});
		idx = result >= 0 ? node._front : node._back;
	}
	return idx;
}

void Interior::loadShape(const Common::String &shapeName, const Common::Array<uint8> &materialMapping) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{shapeName}));
	stream->skip(4 + 6 + 12);
	auto version = stream->readUint32LE();
	stream->readUint32LE();
	auto textureScale = stream->readFloatLE();
	auto minBounds = readVec3(stream.get());
	auto maxBounds = readVec3(stream.get());
	auto numSurfaces = stream->readUint32LE();
	auto numBSPNodes = stream->readUint32LE();
	auto numBSPLeaves = stream->readUint32LE();
	auto blobSize = stream->readUint32LE();
	auto numVertices = stream->readUint32LE();
	auto numPoints = stream->readUint32LE();
	auto numTexCoords = stream->readUint32LE();
	auto numPlanes = stream->readUint32LE();
	_boundingBox = {minBounds, maxBounds};

	_surfaces.resize(numSurfaces);
	for (int i = 0; i < numSurfaces; i++) {
		auto &s = _surfaces[i];
		uint16 pad;
		stream->readMultipleLE(
			s._flags, s._material,
			s._texScaleX, s._texScaleY,
			s._texOffsetX, s._texOffsetY,
			s._planeIdx, s._vertIdx, s._pointIdx,
			s._numVertices, s._numPoints, pad);
		s._material = materialMapping[s._material];
	}

	_bspNodes.resize(numBSPNodes);
	for (int i = 0; i < numBSPNodes; i++) {
		auto &n = _bspNodes[i];
		stream->readMultipleLE(n._planeIndex, n._front, n._back, n._fill);
	}

	_bspLeaves.resize(numBSPLeaves);
	for (int i = 0; i < numBSPLeaves; i++) {
		auto &l = _bspLeaves[i];
		l._type = BSPLeaf::Type(stream->readUint16LE());
		stream->readMultipleLE(l._pvsNum, l._pvsOffset, l._numSurfaces, l._numPlanes, l._numA, l._numB, l._offsetA, l._offsetB);
		l._minBounds = readVec3(stream.get());
		l._maxBounds = readVec3(stream.get());
	}

	stream->skip(blobSize);

	_vertices.resize(numVertices);
	for (int i = 0; i < numVertices; i++) {
		auto &v = _vertices[i];
		stream->readMultipleLE(v._pointIdx, v._texCoordIdx);
	}

	_points.reserve(numPoints);
	for (int i = 0; i < numPoints; i++) {
		_points.emplace_back(readVec3(stream.get()));
	}

	_texCoords.reserve(numTexCoords);
	for (int i = 0; i < numTexCoords; i++) {
		_texCoords.emplace_back(readVec2(stream.get()));
	}

	_planes.reserve(numPlanes);
	for (int i = 0; i < numPlanes; i++) {
		_planes.emplace_back(readVec4(stream.get()));
	}
	auto highestMip = stream->readSint32LE();
	auto flags = stream->readUint32LE();
}

} // namespace Kq8