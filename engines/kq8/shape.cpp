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

#include "kq8/shape.h"

#include "kq8.h"

namespace Kq8 {
static Math::Vector3d readVec3(Common::SeekableReadStream *stream) {
	Math::Vector3d result;
	stream->readMultipleLE(result.x(), result.y(), result.z());
	return result;
}

// Read a float[9] and vec3 from stream and return a column-major transform.
static Math::Matrix4 readMat4(Common::SeekableReadStream *stream) {
	/* flags = */ stream->readUint32LE();
	Math::Matrix4 transform;
	stream->readMultipleLE(transform(0, 0), transform(0, 1), transform(0, 2));
	stream->readMultipleLE(transform(1, 0), transform(1, 1), transform(1, 2));
	stream->readMultipleLE(transform(2, 0), transform(2, 1), transform(2, 2));
	stream->readMultipleLE(transform(3, 0), transform(3, 1), transform(3, 2));
	transform(3, 3) = 1;
	return transform;
}

static Math::Matrix4 transformFromScaleOrigin(const Math::Vector3d &scale, const Math::Vector3d &origin) {
	Math::Matrix4 transform;
	transform(0, 0) = scale.x();
	transform(1, 1) = scale.y();
	transform(2, 2) = scale.z();
	transform(3, 3) = 1;

	transform(3, 0) = origin.x();
	transform(3, 1) = origin.y();
	transform(3, 2) = origin.z();
	return transform;
}

static Common::String readBoundedString(Common::SeekableReadStream *stream, uint32 len) {
	char *buffer = new char[len + 1];
	stream->read(buffer, len);
	auto ret = Common::String(buffer);
	buffer[len] = '\0';
	delete buffer;
	return ret;
}

Shape *Shape::loadShape(const Common::String &path) {
	Common::ScopedPtr<Shape> shape{new Shape()};
	shape->_name = path;

	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path(path)));
	if (!stream) {
		warning("Could not load font '%s'", path.c_str());
		return nullptr;
	}

	auto tag = stream->readUint32BE();
	if (tag != MKTAG('P', 'E', 'R', 'S')) {
		warning("wrong tag for font, expected PERS");
		return nullptr;
	}

	stream->skip(4);
	uint32 class_len = stream->readUint16LE();
	auto shapeClass = readBoundedString(stream.get(), class_len);
	if (shapeClass == "TS::Shape") {
		warning("No support for TS::Shape");
		return nullptr;
	}

	auto version = stream->readUint32LE();

	shape->_boundingSphere._radius = stream->readFloatLE();
	shape->_boundingSphere._center = readVec3(stream.get());

	auto num_nodes = stream->readUint32LE();
	auto num_sequences = stream->readUint32LE();
	auto num_a48 = stream->readUint32LE();
	auto num_a52 = stream->readUint32LE();
	auto num_a56 = stream->readUint32LE();
	auto num_names = stream->readUint32LE();
	auto num_meshes = stream->readUint32LE();
	auto num_transitions = version >= 2 ? stream->readUint32LE() : 0;

	shape->_nodes.resize(num_nodes);
	for (int i = 0; i < num_nodes; i++) {
		Node &node = shape->_nodes[i];
		node._transform = readMat4(stream.get());
		node._mesh = stream->readUint16LE();
		node._frame = stream->readUint16LE();
		node._unk3 = stream->readUint16LE();
		node._unk4 = stream->readUint16LE();
	}

	for (int i = 0; i < num_sequences; i++) {
		auto index = stream->readUint16LE();
		auto count = stream->readUint16LE();
		shape->_sequences.push_back(Sequence{index, count});
	}

	for (int i = 0; i < num_a48; i++) {
		stream->skip(56);
	}

	/* auto stuff = */ stream->skip(12);

	/* stuff_from_a52 */ stream->skip(4 * num_a52);
	/* stuff2 = */ stream->skip(4);

	for (int i = 0; i < num_names; i++) {
		stream->skip(24);
	}

	for (int i = 0; i < num_transitions; i++) {
		stream->skip(version >= 4 ? 36 : 28);
	}

	shape->_meshes.resize(num_meshes);
	for (int m = 0; m < num_meshes; m++) {
		Mesh &mesh = shape->_meshes[m];
		stream->skip(26);
		auto mesh_version = stream->readUint32LE();
		auto num_verts = stream->readUint32LE();
		auto verts_per_frame = stream->readUint32LE();
		auto num_texture_verts = stream->readUint32LE();
		auto num_faces = stream->readUint32LE();
		auto num_frames = stream->readUint32LE();
		auto texture_verts_per_frame = mesh_version >= 2 ? stream->readUint32LE() : 0;
		auto mesh_scale = mesh_version < 3 ? readVec3(stream.get()) : Math::Vector3d{1, 1, 1};
		auto mesh_origin = mesh_version < 3 ? readVec3(stream.get()) : Math::Vector3d{0, 0, 0};
		auto mesh_transform = transformFromScaleOrigin(mesh_scale, mesh_origin);
		auto radius = stream->readFloatLE();

		mesh._packedVertices.resize(4 * num_verts);
		stream->read(mesh._packedVertices.data(), mesh._packedVertices.size());

		mesh._texcoords.resize(num_texture_verts);
		for (int i = 0; i < num_texture_verts; i++) {
			stream->readMultipleLE(mesh._texcoords[i]._u, mesh._texcoords[i]._v);
		}

		mesh._faces.resize(num_faces);
		for (int i = 0; i < num_faces; i++) {
			Face &face = mesh._faces[i];
			stream->readMultipleLE(face._verts[0], face._texcoords[0]);
			stream->readMultipleLE(face._verts[1], face._texcoords[1]);
			stream->readMultipleLE(face._verts[2], face._texcoords[2]);
			stream->readMultipleLE(face._material);
		}

		Common::sort(mesh._faces.begin(), mesh._faces.end(),
					 [](const Face &a, const Face &b) { return a._material < b._material; });

		mesh._frames.resize(num_frames);
		for (int i = 0; i < num_frames; i++) {
			Frame &frame = mesh._frames[i];
			frame._firstVertex = stream->readUint32LE();
			if (mesh_version >= 3) {
				auto scale = readVec3(stream.get());
				auto origin = readVec3(stream.get());
				frame._transform = transformFromScaleOrigin(scale, origin);
			} else {
				frame._transform = mesh_transform;
			}
		}
	}

	uint32 has_material = stream->readUint32LE();
	if (has_material) {
		stream->skip(26);
		auto materialListVersion = stream->readUint32LE();
		stream->skip(4);
		auto num_materials = stream->readUint32LE();
		auto *objectPalette = g_engine->world()->getObjectPalette();

		for (int i = 0; i < num_materials; i++) {
			stream->skip(16);
			auto fileName = readBoundedString(stream.get(), materialListVersion < 2 ? 16 : 32);
			auto &graphicsManager = g_engine->graphicsManager();
			auto *bitmap =
				graphicsManager.loadBitmap(fileName, objectPalette, GraphicsManager::BitmapPacking::kLoose);
			shape->_materials.emplace_back(bitmap);
		}
	}

	return shape.release();
}

} // namespace Kq8