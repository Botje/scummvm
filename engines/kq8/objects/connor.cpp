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

void Connor::update(float dt) {
	auto inputs = g_engine->inputs();
	if (inputs & Input::kRight) {
		_rot.z() -= M_PI / 2 * dt;
	}
	if (inputs & Input::kLeft) {
		_rot.z() += M_PI / 2 * dt;
	}
	if (inputs & Input::kForward) {
		auto delta = getTransform() * Math::Vector4d{0, -30, 0, 0};
		_pos += delta.getXYZ();
	}
	if (inputs & Input::kBackward) {
		auto delta = getTransform() * Math::Vector4d{0, 30, 0, 0};
		_pos += delta.getXYZ();
	}
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

} // namespace Kq8