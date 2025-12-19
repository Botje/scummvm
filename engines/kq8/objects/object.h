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
#include "kq8/shape.h"

namespace Kq8 {

class Object : private Common::NonCopyable {
protected:
	Common::String _classType;
	Math::Vector3d _pos = {0, 0, 0};
	Math::Vector3d _rot = {0, 0, 0};
	Common::String _name;

	Shape *_shape = nullptr;

public:
	static Object *factory(const KQFile &f);
	virtual ~Object() = default;
	Object(const KQFile &f);
	const Common::String &classType() const { return _classType; }
	void moveTo(const Math::Vector3d &pos) { _pos = pos; }
	const Math::Vector3d &pos() const { return _pos; }
	void setName(const Common::String &name) { _name = name; }
	const Common::String &name() const { return _name; }
	void setRotation(const Math::Vector3d &rot) { _rot = rot; }
	Math::Vector3d rot() const { return _rot; }
	void draw();
};

} // namespace Kq8

#endif // OBJECT_H
