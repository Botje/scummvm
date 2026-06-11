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

#include "kq8/objects/camera.h"

#include "kq8/kq8.h"

namespace Kq8 {

void Camera::update(float dt) {
	Object::update(dt);

	if (_following.empty()) {
		auto forward = Math::Vector3d{0, 1, 0};
		auto up = Math::Vector3d{0, 0, 1};
		auto q = Math::Matrix4{
			Math::Angle::fromRadians(rot().z()),
			Math::Angle::fromRadians(rot().x()),
			Math::Angle::fromRadians(rot().y()),
			Math::EO_ZXY};
		q.transform(&forward, false);
		q.transform(&up, false);
		auto right = Math::Vector3d::crossProduct(forward, up);
		_direction = forward;
		_up = up;
		_right = right;
	} else {
		auto *target = g_engine->world()->findObject(_following);
		if (!target)
			return;

		auto toTarget = (target->pos() - pos());
		toTarget.z() = 0;
		auto distance = toTarget.length();
		if (distance > _maxDistance) {
			pos() += (distance - _maxDistance) * toTarget.getNormalized();
		} else if (distance < _minDistance) {
			pos() -= (_minDistance - distance) * toTarget.getNormalized();
		}
		auto focus = Math::Vector3d{0, 0, target->boundingBox()._max.z()};
		target->getTransform().transform(&focus, true);

		_direction = (focus - pos()).getNormalized();
		_right = Math::Vector3d::crossProduct(_direction, Math::Vector3d{0, 0, 1});
		_up = Math::Vector3d::crossProduct(_right, _direction);
	}
}

Math::Matrix4 Camera::getCamMatrix() const {
	auto undoCamera = Math::Matrix4{};
	undoCamera.setToIdentity();
	undoCamera.setPosition(-pos());

	auto worldToCam = Math::Matrix4{};
	worldToCam.getRow(0) << _right.x() << _right.y() << _right.z() << 0;
	worldToCam.getRow(1) << _direction.x() << _direction.y() << _direction.z() << 0;
	worldToCam.getRow(2) << _up.x() << _up.y() << _up.z() << 0;
	worldToCam.getRow(3) << 0 << 0 << 0 << 1;

	return worldToCam * undoCamera;
}
void Camera::zoomIn() {
	auto *target = g_engine->world()->findObject(_following);
	if (!target)
		return;

	auto toTarget = (target->pos() - pos());
	if (toTarget.length() > _minDistance) {
		pos() += 100 * toTarget.getNormalized();
	}
}

void Camera::zoomOut() {
	auto *target = g_engine->world()->findObject(_following);
	if (!target)
		return;

	auto toTarget = (target->pos() - pos());
	if (toTarget.length() < _maxDistance) {
		pos() -= 100 * toTarget.getNormalized();
	}
}
void Camera::pan(Math::Vector2d delta) {
	if (_following.empty()) {
		rot().z() += delta.getX();
		rot().x() += delta.getY();
	} else {
		pos() += 1000 * delta.getX() * _right + 1000 * delta.getY() * _up;
	}
}
void Camera::saveToStream(CBOR::WriteStream &out) const {
	Object::saveToStream(out);
	out << "following" << _following;
	out << "direction" << _direction;
	out << "right" << _right;
	out << "up" << _up;
	out << "minDistance" << _minDistance;
	out << "maxDistance" << _maxDistance;
}

Object::AttributeSetters Camera::attributesToLoad() const {
#define OBJ static_cast<Camera *>(it)
	auto ret = Object::attributesToLoad();
	ret["following"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_following = in.readString(); };
	ret["direction"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_direction = in.readVector3d(); };
	ret["right"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_right = in.readVector3d(); };
	ret["up"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_up = in.readVector3d(); };
	ret["minDistance"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_minDistance = in.readFloat(); };
	ret["maxDistance"] = [](Object *it, CBOR::ReadStream &in) { OBJ->_maxDistance = in.readFloat(); };
	return ret;
#undef OBJ
}

} // namespace Kq8