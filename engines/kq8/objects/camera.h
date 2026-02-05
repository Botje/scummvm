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

#ifndef KQ8_OBJECTS_CAMERA_H
#define KQ8_OBJECTS_CAMERA_H

#include "common/rect.h"
#include "kq8/objects/object.h"
#include "kq8/world.h"
#include "math/vector2d.h"

namespace Kq8 {

class Camera : public Object {
public:
	Camera() : Object{"KQCamera"} {}
	virtual bool addToWorld() { return false; }
	void update(float dt) override;
	void follow(const Common::String &target) { _following = target; }

	const Common::String &following() const { return _following; }
	Math::Vector3d direction() const { return _direction; }
	Math::Matrix4 getCamMatrix() const;
	void zoomIn();
	void zoomOut();
	void pan(Math::Vector2d delta);

private:
	Common::String _following = "";
	Math::Vector3d _direction{0, 1, 0};
	Math::Vector3d _right{1, 0, 0};
	Math::Vector3d _up{0, 0, 1};
	float _minDistance = 1000;
	float _maxDistance = 5000;
};

} // namespace Kq8

#endif // KQ8_OBJECTS_CAMERA_H
