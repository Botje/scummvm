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

#ifndef SCUMMVM_KQ8_CBOR_H
#define SCUMMVM_KQ8_CBOR_H

#include "common/span.h"
#include "common/stream.h"
#include "math/vector3d.h"

namespace Kq8 {
namespace CBOR {

enum class Token : uint8 {
	Invalid = 0,
	UnsignedInt,
	SignedInt,
	ByteString,
	UTF8String,
	Array,
	Map,
	Tag,
	Float16,
	Float,
	Double,
	Break,
	False,
	True,
	Null,
	Undefined,
};

struct WithArgument {
	Token _token;
	uint32 _size;
};

struct ByteString {
	Common::Span<byte> _bytes;
};

class WriteStream : private Common::NonCopyable {
public:
	WriteStream(Common::WriteStream &stream) : _stream(stream) {}
	WriteStream &operator<<(Token t);
	WriteStream &operator<<(const WithArgument &s);
	WriteStream &operator<<(uint32 i);
	WriteStream &operator<<(int32 i);
	WriteStream &operator<<(float f);
	WriteStream &operator<<(const Common::String &s);
	WriteStream &operator<<(const Math::Vector3d &v);
	WriteStream &operator<<(const ByteString &bs);

private:
	Common::WriteStream &_stream;
};

class ReadStream : private Common::NonCopyable {
public:
	ReadStream(Common::ReadStream &stream) : _stream(stream) {}
	Token nextToken();
	void expect(Token expected);
	uint32 readUInt();
	Common::String readString();

private:
	Common::ReadStream &_stream;
};

} // End of namespace CBOR
} // End of namespace Kq8
#endif // SCUMMVM_KQ8_CBOR_H
