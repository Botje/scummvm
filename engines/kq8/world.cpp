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

void World::update(float dt) {
	for (auto *object : _objects) {
		object->update(dt);
	}

	if (!_toDelete.empty()) {
		auto it = _objects.begin();
		while (it != _objects.end() && !_toDelete.empty()) {
			auto toDeleteIt = Common::find(_toDelete.begin(), _toDelete.end(), *it);
			if (toDeleteIt != _toDelete.end()) {
				*it = _objects.back();
				_objects.pop_back();
				_toDelete.erase(toDeleteIt);
			} else {
				++it;
			}
		}
		_toDelete.clear();
	}
}

void World::draw() {
	if (_terrain) {
		g_engine->gfx().drawTerrain(_terrain);
	}

	auto visibleDistance = _terrain->visibleDistance();
	auto camPos = g_engine->camera()->pos();

	for (auto *object : _objects) {
		// auto toObject = object->pos() - camPos;
		// auto dot = camera()->direction().dotProduct(toObject);
		// if (dot < 0 || dot > visibleDistance * visibleDistance) {
		// 	continue;
		// }
		object->draw();
	}
}

Object *World::findObject(const Common::String &name) {
	if (name == "KQCamera")
		return g_engine->camera();
	if (name == "Connor")
		return g_engine->connor();

	for (auto *object : _objects) {
		if (object->name() == name) {
			return object;
		}
	}
	return nullptr;
}
void World::deleteLater(Object *obj) {
	_toDelete.push_back(obj);
}

void BVHTree::updateNodeBoundingBox(BVHNode &node) {
	node.aabb = {{1e30, 1e30, 1e30}, {-1e30, -1e30, -1e30}};
	for (int i = node.first; i < node.first + node.count; i++) {
		const auto &bbox = _aabbs[_objects[i]];
		node.aabb.extend(bbox._min).extend(bbox._max);
	}
}

template<class T, class F>
uint count_if(T *begin, T *end, F &&pred) {
	uint ret = 0;
	while (begin != end) {
		if (pred(*begin))
			ret++;
		++begin;
	}
	return ret;
}

Common::Pair<int, float> BVHTree::determineSplitPlane(const BVHNode &node) {
	int bestAxis = -1;
	float bestPos = 0;
	int bestScore = node.count / 2;

	for (int axis = 0; axis < 2; axis++) {
		float min = node.aabb._min.getValue(axis);
		float max = node.aabb._max.getValue(axis);
		float scale = (max - min) / 20;
		for (int i = 1; i < 20; i++) {
			float splitPos = min + scale * i;
			auto leftSplitCount = count_if(_objects.begin() + node.first, _objects.begin() + node.first + node.count, [=](Object *obj) {
				return obj->pos().getValue(axis) < splitPos;
			});
			if (leftSplitCount == node.count / 2)
				return {axis, splitPos};
			auto score = abs(int(node.count / 2) - int(leftSplitCount));
			if (score < bestScore) {
				bestAxis = axis;
				bestPos = splitPos;
				bestScore = score;
			}
		}
	}

	return {bestAxis, bestPos};
}

void BVHTree::subdivide(BVHNode &node) {
	if (node.count <= 2)
		return;
	auto p = determineSplitPlane(node);
	if (p.first == -1)
		return;

	auto leftCount = partition(p.first, p.second, node);
	if (leftCount == 0 || leftCount == node.count)
		return;

	node.left.reset(new BVHNode(node.first, leftCount));
	updateNodeBoundingBox(*node.left);
	subdivide(*node.left);

	auto rightCount = node.count - leftCount;
	node.right.reset(new BVHNode(node.first + leftCount, rightCount));
	updateNodeBoundingBox(*node.right);
	subdivide(*node.right);

	node.first = node.count = 0;
}

void BVHNode::print(uint indent) {
	Common::String s;
	for (int i = 0; i < indent; i++)
		s += "  ";
	debug("%s(%f, %f) -> (%f, %f) count=%d", s.c_str(), aabb._min.x(), aabb._min.y(), aabb._max.x(), aabb._max.y(), count);
	if (left)
		left->print(indent + 1);
	if (right)
		right->print(indent + 1);
}

void BVHTree::print() {
	_root.print(0);
}

Object *BVHTree::findEnclosingObject(const Math::Vector3d &pos) const {
	Common::Queue<const BVHNode *> todo;
	todo.push(&_root);
	while (!todo.empty()) {
		auto *node = todo.pop();
		if (node->aabb.contains(pos)) {
			if (node->isLeaf()) {
				for (int i = node->first; i < node->first + node->count; i++) {
					auto *candidate = _objects[i];
					if (_aabbs[candidate].contains(pos)) {
						return candidate;
					}
				}
			} else {
				if (node->left)
					todo.push(node->left.get());
				if (node->right)
					todo.push(node->right.get());
			}
		}
	}
	return nullptr;
}

uint BVHTree::partition(uint axis, float splitPos, const BVHNode &node) {
	auto left = &_objects[node.first];
	auto right = &_objects[node.first + node.count - 1];
	auto pred = [=](Object *obj) {
		return obj->pos().getValue(axis) < splitPos;
	};

	while (left != right && pred(*left))
		++left;

	if (left != right) {
		for (auto i = left + 1; i != right; ++i) {
			if (pred(*i)) {
				SWAP(*i, *left);
				++left;
			}
		}
	}
	return left - &_objects[node.first];
}

BVHTree::BVHTree(const Common::Array<Object *> &objects) : _root(0, objects.size()), _objects{objects} {
	for (auto *object : objects) {
		_aabbs[object] = object->aabb();
	}

	updateNodeBoundingBox(_root);
	subdivide(_root);
}

void World::updateBVH() {
	Common::Array<Object *> interiors;
	for (auto *object : _objects) {
		auto *interior = dynamic_cast<Interior *>(object);
		if (!interior)
			continue;
		interiors.push_back(interior);
	}
	auto interiorTree = BVHTree{interiors};

	auto topLevelObjects = interiors;
	for (auto *object : _objects) {
		if (dynamic_cast<Monster *>(object) || dynamic_cast<Interior *>(object))
			continue;

		auto *parent = interiorTree.findEnclosingObject(object->pos());
		if (!parent) {
			debug("Found top-level entity %s %s", object->classType().c_str(), object->name().c_str());
			topLevelObjects.push_back(object);
		}
	}

	_bvhTree = BVHTree{topLevelObjects};
}

CBOR::WriteStream &operator<<(CBOR::WriteStream &out, const World &world) {
	using namespace CBOR;

	Common::MemoryWriteStreamDynamic temp{DisposeAfterUse::YES};
	CBOR::WriteStream inner{temp};
	inner << WithArgument{Token::Array, 1 + world._objects.size()};
	inner << *world._terrain;
	for (const auto *obj : world._objects) {
		inner << *obj;
	}

	out << ByteString{{temp.getData(), uint32(temp.size())}};
	return out;
}

} // namespace Kq8