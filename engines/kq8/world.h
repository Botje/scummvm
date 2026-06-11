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

#include "common/hash-ptr.h"
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

struct BVHNode {
	Object::BoundingBox aabb;
	Common::ScopedPtr<BVHNode> left;
	Common::ScopedPtr<BVHNode> right;
	uint16 first;
	uint16 count;
	bool isLeaf() const { return count > 0; }
	void print(uint indent);
	BVHNode() = default;
	BVHNode(uint16 first, uint16 count)
		: first{first},
		  count{count} {}
};
class BVHTree {
private:
	using AABBs = Common::HashMap<Object *, Object::BoundingBox>;
	AABBs _aabbs;
	BVHNode _root;
	Common::Array<Object *> _objects;
	void updateNodeBoundingBox(BVHNode &node);
	Common::Pair<int, float> determineSplitPlane(const BVHNode &node);
	uint partition(uint axis, float splitPos, const BVHNode &node);
	void subdivide(BVHNode &node);
	void print(uint indent, BVHNode &node);

public:
	BVHTree() = default;
	BVHTree(const Common::Array<Object *> &objects);
	void print();
	Object *findEnclosingObject(const Math::Vector3d &pos) const;
};

class World {
public:
	World(const Common::String &name);
	~World();

	Terrain *terrain() { return _terrain; }
	void setTerrain(Terrain *terrain) { _terrain = terrain; }
	const Common::String &name() const { return _name; }
	const Graphics::Palette *getObjectPalette();
	void addObject(Object *object);
	void update(float dt);
	void draw();
	Object *findObject(const Common::String &name);
	void deleteLater(Object *obj);
	void updateBVH();
	Object *findEnclosingObject(const Math::Vector3d &pos) const { return _bvhTree.findEnclosingObject(pos); }

	friend CBOR::WriteStream &operator<<(CBOR::WriteStream &out, const World &world);

private:
	Common::String _name;
	Terrain *_terrain = nullptr;
	Common::Array<Object *> _objects;
	Common::Array<Object *> _toDelete;
	BVHTree _bvhTree;
};

} // namespace Kq8

#endif // WORLD_H
