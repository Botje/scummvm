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

#ifndef KQ8_SHAPE_H
#define KQ8_SHAPE_H

#include "common/array.h"
#include "common/str.h"
#include "math/matrix4.h"

namespace Kq8 {

class GraphicsManager;
class Bitmap;

class Shape {
	friend class GraphicsManager;

public:
	struct Node {
		uint32 _flags;
		Math::Matrix4 _transform;
		uint16 _mesh;
		uint16 _frame;
		uint16 _unk3;
		uint16 _unk4;
	};

	struct SubSequence {
		uint16 _nodeIndex;
		uint16 _nodeCount;
	};

	struct Sequence {
		Math::Matrix4 _transform;
		int16 _subsequenceIndex;
		int16 _lightSubsequenceIndex;
	};

	struct TexCoord {
		float _u;
		float _v;
	};

	struct Face {
		uint32 _verts[3];
		uint32 _texcoords[3];
		uint32 _material;
	};

	struct Frame {
		uint32 _firstVertex;
		Math::Matrix4 _transform;
	};

	struct Mesh {
		Common::Array<uint8> _packedVertices;
		Common::Array<TexCoord> _texcoords;
		Common::Array<Face> _faces;
		Common::Array<Frame> _frames;
	};

	struct Loop {
		int16 sequenceIndex;
		int16 sequenceCount;
		int16 keyframeIndex;
		int16 keyframeCount;
	};

	static Shape *loadShape(const Common::String &path);
	struct {
		float _radius;
		Math::Vector3d _center;
	} _boundingSphere;
	Common::String _name;
	Common::Array<Node> _nodes;
	Common::Array<SubSequence> _subSequences;
	Common::Array<Sequence> _sequences;
	Common::Array<Mesh> _meshes;
	Common::Array<Loop> _loops;
	Common::Array<Bitmap *> _materials;
};

} // namespace Kq8

#endif // KQ8_SHAPE_H
