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

#ifndef KQ8_OBJECTS_OBJECT_H
#define KQ8_OBJECTS_OBJECT_H

#include "common/noncopyable.h"
#include "common/str.h"
#include "math/vector3d.h"

#include "kq8/kq_file.h"
#include "kq8/script.h"
#include "kq8/shape.h"

namespace Kq8 {

class Object : private Common::NonCopyable {
public:
	struct BoundingBox {
		Math::Vector3d _min;
		Math::Vector3d _max;
		friend bool operator==(const BoundingBox &lhs, const BoundingBox &rhs) { return lhs._min == rhs._min && lhs._max == rhs._max; }
		friend bool operator!=(const BoundingBox &lhs, const BoundingBox &rhs) { return !(lhs == rhs); }
		bool contains(const Math::Vector3d &pos) const {
			return _min.x() <= pos.x() && _min.y() <= pos.y() && _min.z() <= pos.z() &&
				   pos.x() <= _max.x() && pos.y() <= _max.y() && pos.z() <= _max.z();
		}

		BoundingBox &extend(const Math::Vector3d &p) {
			_min = Math::Vector3d{MIN(_min.x(), p.x()), MIN(_min.y(), p.y()), MIN(_min.z(), p.z())};
			_max = Math::Vector3d{MAX(_max.x(), p.x()), MAX(_max.y(), p.y()), MAX(_max.z(), p.z())};

			return *this;
		}
	};
	enum { kObjectNoHealth = -1000 };

protected:
	Common::String _classType;
	Math::Vector3d _pos = {0, 0, 0};
	Math::Vector3d _rot = {0, 0, 0};
	Common::String _name;
	Shape *_shape = nullptr;
	Common::String _script;
	BoundingBox _boundingBox;
	uint16 _colliderMask = 0;
	int16 _health = kObjectNoHealth;

protected:
	Object(const Common::String &name) : _name(name) {};

public:
	static Object *factory(const KQFile &f);
	virtual ~Object() = default;

	Object(const KQFile &f, bool tryLoadShape = false);
	const Common::String &classType() const { return _classType; }
	void moveTo(const Math::Vector3d &pos);
	const Math::Vector3d &pos() const { return _pos; }
	Math::Vector3d &pos() { return _pos; }
	void setName(const Common::String &name) { _name = name; }
	const Common::String &name() const { return _name; }
	void setRotation(const Math::Vector3d &rot) { _rot = rot; }
	const Math::Vector3d &rot() const { return _rot; }
	Math::Vector3d &rot() { return _rot; }
	Common::String script() const { return _script; }
	void setScript(const Common::String &script) { _script = script; }
	Math::Matrix4 getTransform() const;
	const BoundingBox &boundingBox() const { return _boundingBox; };
	void setBoundingBox(const BoundingBox &bbox) { _boundingBox = bbox; }
	BoundingBox aabb() const;
	uint16 colliderMask() const { return _colliderMask; }
	void updateColliderMask(uint16 flags, bool toAdd);
	int16 health() const { return _health; }
	void adjustHealth(int16 delta);

	virtual bool addToWorld() { return true; }
	virtual bool canInteract() const { return !_script.empty(); }
	virtual void draw();
	virtual void update(float dt) {}
	virtual void sendEvent(const Common::String &eventType, const Script::Args &args);
	virtual bool collide(Object *collider, const Math::Vector3d &newPos) { return true; }
};

} // namespace Kq8

#endif // OBJECT_H
