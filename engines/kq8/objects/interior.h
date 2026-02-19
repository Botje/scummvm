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

#ifndef KQ8_OBJECTS_INTERIOR_H
#define KQ8_OBJECTS_INTERIOR_H

#include "connor.h"
#include "kq8/objects/object.h"
#include "math/vector2d.h"
#include "math/vector4d.h"

namespace Kq8 {

class Interior : public Object {
public:
	struct Surface {
		uint8 _flags;
		uint8 _material;
		uint8 _texScaleX;
		uint8 _texScaleY;
		uint8 _texOffsetX;
		uint8 _texOffsetY;
		uint16 _planeIdx;
		uint32 _vertIdx;
		uint32 _pointIdx;
		uint8 _numVertices;
		uint8 _numPoints;
	};

	struct Vertex {
		uint16 _pointIdx;
		uint16 _texCoordIdx;
	};

	struct BSPNode {
		uint16 _planeIndex;
		// Negative numbers refer to leaves, positive to nodes
		int16 _front;
		int16 _back;
		int16 _fill;
	};

	struct BSPLeaf {
		enum Type {
			kLeafType0 = 0,
			kLeafTypeOutside = 1,
			kLeafType2 = 2,
		} _type;
		uint16 _pvsNum;
		uint32 _pvsOffset;
		uint16 _numSurfaces;
		uint16 _numPlanes;
		uint16 _numA;
		uint16 _numB;
		uint32 _offsetA;
		uint32 _offsetB;
		Math::Vector3d _minBounds;
		Math::Vector3d _maxBounds;
	};

	static Object *factory(const KQFile &f);
	Interior(const KQFile &f);
	void draw() override;

	const Common::Array<Surface> &surfaces() const { return _surfaces; }
	const Common::Array<Vertex> &vertices() const { return _vertices; }
	const Common::Array<Math::Vector3d> &points() const { return _points; }
	const Common::Array<Math::Vector2d> &texCoords() const { return _texCoords; }
	const Common::Array<Bitmap *> &materials() const { return _materials; }
	const Common::Array<BSPLeaf> &bspLeaves() const { return _bspLeaves; }
	int16 evaluateBSP(const Math::Vector3d &pos);

private:
	void loadShape(const Common::String &shapeName, const Common::Array<uint8> &materialMapping);

	Common::String _shapeName;
	Common::Array<Surface> _surfaces;
	Common::Array<Vertex> _vertices;
	Common::Array<Math::Vector3d> _points;
	Common::Array<Math::Vector2d> _texCoords;
	Common::Array<Bitmap *> _materials;
	Common::Array<BSPNode> _bspNodes;
	Common::Array<BSPLeaf> _bspLeaves;
	Common::Array<Math::Vector4d> _planes;
};

} // namespace Kq8

#endif // KQ8_OBJECTS_INTERIOR_H
