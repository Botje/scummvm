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

#ifndef KQ8_READ_HELPERS_H
#define KQ8_READ_HELPERS_H

#include "common/archive.h"
#include "common/str.h"
#include "math/matrix4.h"
#include "math/vector2d.h"
#include "math/vector3d.h"

static inline Math::Vector2d readVec2(Common::SeekableReadStream *stream) {
	float x;
	float y;
	stream->readMultipleLE(x, y);
	return Math::Vector2d{x, y};
}

static inline Math::Vector3d readVec3(Common::SeekableReadStream *stream) {
	Math::Vector3d result;
	stream->readMultipleLE(result.x(), result.y(), result.z());
	return result;
}

static inline Math::Vector4d readVec4(Common::SeekableReadStream *stream) {
	Math::Vector4d result;
	stream->readMultipleLE(result.x(), result.y(), result.z(), result.w());
	return result;
}

static inline Common::String readBoundedString(Common::SeekableReadStream *stream, uint32 len) {
	char *buffer = new char[len + 1];
	stream->read(buffer, len);
	buffer[len] = '\0';
	auto ret = Common::String(buffer);
	delete[] buffer;
	return ret;
}

#endif // KQ8_READ_HELPERS
