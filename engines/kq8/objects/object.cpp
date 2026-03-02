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

#include "kq8/objects/object.h"

#include "kq8/kq8.h"

namespace Kq8 {

Object *Object::factory(const KQFile &f) {
	return new Object(f, true);
}

Object::Object(const KQFile &f, bool tryLoadShape) {
	using namespace INIHelpers;
	auto &section = f.getSections().front();
	_name = section.name;
	_classType = section.getKey("classType")->value;
	if (section.hasKey("script")) {
		_script = section.getKey("script")->value;
		g_engine->queueScript(_script, Script::Args{_name, "Init"});
	}

	auto shapeName = section.getKey("shapeName");
	if (tryLoadShape && shapeName) {
		_shape = g_engine->graphicsManager().loadshape(shapeName->value);
	}

	Math::Vector3d pos = get(section, "locX", "locY", "locZ");
	moveTo(pos);

	Math::Vector3d rot = get(section, "dirX", "dirY", "dirZ");
	setRotation(rot);
}
void Object::moveTo(const Math::Vector3d &pos) {
	_pos = pos;
	_trackGround = pos.z() == -1;
}

Math::Matrix4 Object::getTransform() const {
	auto m = Math::Matrix4{Math::Angle::fromRadians(_rot.z()), Math::Angle::fromRadians(_rot.y()), Math::Angle::fromRadians(_rot.x()), Math::EulerOrder::EO_ZYX};
	m.setPosition(_pos);
	return m;
}

void Object::draw() {
	if (!_shape)
		return;
	Math::Matrix4 objectTransform = getTransform();
	g_engine->gfx().drawShape(this, _shape, objectTransform);
}
void Object::update(float dt) {
	if (_trackGround) {
		auto *terrain = g_engine->world()->terrain();
		if (terrain) {
			float adaptedZ = terrain->adaptZ(_pos.x(), _pos.y());
			_pos.z() = adaptedZ;
		}
	}
}

void Object::sendEvent(const Common::String &eventType, const Script::Args &args) {
	if (_script.empty()) {
		return;
	}

	Script::Args eventArgs{_name, eventType};
	for (const auto &arg : args) {
		eventArgs.push_back(arg);
	}

	g_engine->runScript(_script, eventArgs);
}
} // namespace Kq8
