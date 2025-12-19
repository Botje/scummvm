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
#include "kq8/kq_file.h"

namespace Kq8 {

class Object : private Common::NonCopyable {
protected:
	Common::String _classType;

public:
	virtual ~Object() = default;
	Object(const KQFile &f) {
		auto &section = f.getSections().front();
		_classType = section.getKey("classType")->value;
	}
	const Common::String &classType() const { return _classType; }
};

} // namespace Kq8

#endif // OBJECT_H
