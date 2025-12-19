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
Object::Object(const KQFile &f) {
	auto &section = f.getSections().front();
	_classType = section.getKey("classType")->value;

	auto shapeName = section.getKey("shapeName");
	if (shapeName) {
		_shape = g_engine->graphicsManager().loadshape(shapeName->value);
	}
}
void Object::draw() {
	if (!_shape)
		return;
	Math::Matrix4 objectTransform;
	objectTransform.setToIdentity();
	objectTransform(3, 0) = _pos.x();
	objectTransform(3, 1) = _pos.y();
	objectTransform(3, 2) = _pos.z();
	g_engine->gfx().drawShape(_shape, 0, objectTransform);
}
} // namespace Kq8