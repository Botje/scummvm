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

#include "kq8/objects/connor.h"

#include "kq8/kq8.h"

namespace Kq8 {

Object *Connor::factory(const KQFile &f) {
	return new Connor(f);
}
Connor::Connor(const KQFile &f) : Monster{f} {
	_colliderMask = ~0;
	loadAnimLoopFromFile("conner.anm");

	auto connorSection = f.getSections().front();
	auto *inventorySectionName = connorSection.getKey("inventory");
	auto *inventorySection = inventorySectionName ? f.getSection(inventorySectionName->value) : nullptr;
	if (!inventorySection)
		return;

	using namespace INIHelpers;
	Common::Array<Common::String> itemSectionNames = getArray<Common::String>(*inventorySection, "numItems", "ItemName");
	for (const auto &itemSectionName : itemSectionNames) {
		auto *itemSection = f.getSection(itemSectionName);
		const auto itemType = get<Common::String>(*itemSection, "itemType");
		const auto quantity = get<int>(*itemSection, "quantity");
		Connor::addToInventory(g_engine->reference().itemType(itemType), quantity);
	}
}

void Connor::startSpecialAnimation(const Common::String &animListName, const Common::Array<Common::String> &loops) {
	auto *loopList = g_engine->graphicsManager().loadAnimationLoopList(animListName);
	debug("%s: starting animation list", name().c_str());
	_specialAnimation.reset(new AnimationSequence{loopList, this, loops});
}

Math::Vector3d Connor::desiredMovementFromInput(float dt, uint32 inputs) {
	Math::Vector3d deltaPos;
	if (inputs & Input::kRight) {
		_rot.z() -= M_PI / 2 * dt;
	} else if (inputs & Input::kLeft) {
		_rot.z() += M_PI / 2 * dt;
	}
	const auto speed_s = 2000;
	if (inputs & Input::kForward) {
		auto delta = getTransform() * Math::Vector4d{0, -speed_s * dt, 0, 0};
		deltaPos = delta.getXYZ();
	} else if (inputs & Input::kBackward) {
		auto delta = getTransform() * Math::Vector4d{0, speed_s * dt, 0, 0};
		deltaPos = delta.getXYZ();
	}
	return deltaPos;
}

void Connor::update(float dt) {
	auto inputs = g_engine->inputs();
	auto deltaPos = desiredMovementFromInput(dt, inputs);
	deltaPos += _speed;
	const Math::Vector3d halfHeight{0, 0, (_boundingBox._max.z() - _boundingBox._min.z()) / 2};
	auto newPos = _pos + deltaPos + halfHeight;

	auto *enclosing = g_engine->world()->findEnclosingObject(newPos);
	if (enclosing) {
		bool shouldStop = enclosing->collide(this, newPos);
		if (shouldStop) {
			_speed = {0, 0, 0};
			newPos = _pos;
		}
	}
	// collide with objects close by

	_speed.z() += -16000 * dt;
	newPos.z() += _speed.z() - halfHeight.z();

	// collide with ground
	auto *terrain = g_engine->world()->terrain();
	float terrainZ = terrain->adaptZ(newPos.x(), newPos.y());
	if (newPos.z() <= terrainZ) {
		newPos.z() = terrainZ;
		_speed = {0, 0, 0};
	}

	_pos = newPos;

	if (_specialAnimation) {
		Object::update(dt);
		bool animationFinished = _specialAnimation->advanceAnimation(dt);

		if (animationFinished) {
			debug("%s: advancing animation list (%d remain)", name().c_str(), _specialAnimation->_loopList.size() - 1);
			g_engine->notifyAnimationEnded(this, _specialAnimation->currentLoop()->_name);
			bool finished = _specialAnimation->advanceLoop();
			if (finished) {
				debug("%s: finished animation list", name().c_str());
				_specialAnimation.reset();
			}
		}
	}
	if (!_specialAnimation) {
		if (_lastInputs != inputs) {
			if (inputs & Input::kForward) {
				startAnimation({"walk"}, true);
			} else if (inputs & Input::kBackward) {
				startAnimation({"back"}, true);
			} else if (inputs & Input::kLeft) {
				startAnimation({"left"}, true);
			} else if (inputs & Input::kRight) {
				startAnimation({"right"}, true);
			} else {
				startAnimation({"stop"}, true);
			}
		}
		_lastInputs = inputs;
		AnimObject::update(dt);
	}
}

void Connor::draw() {
	if (_specialAnimation) {
		_specialAnimation->draw(getTransform());
	} else if (_animation) {
		_animation->draw(getTransform());
	}
}

static uint8 ConnorFlagToIndex(const Common::String &flag) {
	if (flag == "BeenToCastle")
		return 1 << 0;
	if (flag == "MetGirlInDC")
		return 1 << 1;
	if (flag == "SageGnomeEarnedTeleport")
		return 1 << 2;
	if (flag == "SawSwampVision")
		return 1 << 3;
	if (flag == "KnowAboutFeather")
		return 1 << 4;
	if (flag == "InTemple4")
		return 1 << 5;
	if (flag == "UsedElevator")
		return 1 << 6;
	return 0;
}

bool Connor::getMultiworldFlag(const Common::String &flag) const {
	auto index = ConnorFlagToIndex(flag);
	return index > 0 && _multiWorldFlags & index;
}

void Connor::setMultiworldFlag(const Common::String &flag, bool enable) {
	auto index = ConnorFlagToIndex(flag);
	if (index == 0)
		return;
	if (enable) {
		_multiWorldFlags |= index;
	} else {
		_multiWorldFlags &= ~index;
	}
}

void Connor::addToInventory(const ItemType *itemType, uint16 quantity) {
	Monster::addToInventory(itemType, quantity);
	g_engine->notifyAddToConnorInventory(itemType, quantity, inventoryCount(itemType));
}
void Connor::removeFromInventory(ItemType *itemType, uint16 quantity) {
	Monster::removeFromInventory(itemType, quantity);
	g_engine->notifyRemoveFromConnorInventory(itemType, quantity, inventoryCount(itemType));
}

uint16 Connor::inventoryCount(const ItemType *item) {
	return _inventory.getValOrDefault(const_cast<ItemType *>(item));
}

} // namespace Kq8