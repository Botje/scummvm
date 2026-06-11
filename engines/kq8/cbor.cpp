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

#include "kq8/cbor.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Kq8 {
namespace CBOR {

WriteStream &WriteStream::operator<<(Token t) {
	switch (t) {
	case Token::Array:
		_stream.writeByte(4 << 5 | 0x1f);
		break;
	case Token::Map:
		_stream.writeByte(5 << 5 | 0x1f);
		break;
	case Token::Break:
		_stream.writeByte(7 << 5 | 0x1f);
		break;
	case Token::False:
		_stream.writeByte(7 << 5 | 20);
		break;
	case Token::True:
		_stream.writeByte(7 << 5 | 21);
		break;
	case Token::Null:
		_stream.writeByte(7 << 5 | 22);
		break;
	case Token::Undefined:
		_stream.writeByte(7 << 5 | 23);
		break;
	default:
		error("Invalid argument %u to CBOR::WriteStream", t);
	}
	return *this;
}

WriteStream &WriteStream::operator<<(const WithArgument &s) {
	switch (s._token) {
	case Token::ByteString:
	case Token::Array:
	case Token::Map:
	case Token::UTF8String:
		break;
	default:
		error("Attempt to write CBOR::WithArgument with wrong major type %u", s._token);
	}
	_stream.writeByte((uint8(s._token) & 0xe0) | 26);
	_stream.writeUint32BE(s._size);
	return *this;
}

WriteStream &WriteStream::operator<<(uint32 i) {
	_stream.writeByte(0 << 5 | 26);
	_stream.writeUint32BE(i);
	return *this;
}

WriteStream &WriteStream::operator<<(int32 i) {
	if (i >= 0) {
		return *this << uint32(i);
	} else {
		_stream.writeByte(1 << 5 | 26);
		_stream.writeUint32BE(1 + abs(i));
		return *this;
	}
}

WriteStream &WriteStream::operator<<(float f) {
	_stream.writeByte(7 << 5 | 26);
	_stream.writeFloatBE(f);
	return *this;
}

WriteStream &WriteStream::operator<<(const Common::String &s) {
	_stream.writeByte(3 << 5 | 24);
	_stream.writeByte(s.size());
	_stream.writeString(s);
	return *this;
}

WriteStream &WriteStream::operator<<(const Math::Vector3d &v) {
	return *this << Token::Array << v.x() << v.y() << v.z() << Token::Break;
}

WriteStream &WriteStream::operator<<(const ByteString &bs) {
	*this << WithArgument{Token::ByteString, uint32(bs._bytes.size())};
	_stream.write(bs._bytes.data(), bs._bytes.size());
	return *this;
}

Token ReadStream::nextToken() {
	byte b = _stream.readByte();
	uint8 major = b >> 5;
	uint8 minor = b & 0x1f;

	switch (major) {
	case 0:
		assert(minor == 26);
		return Token::UnsignedInt;
	case 2:
		error("Unexpected major 2");
	case 1:
		assert(minor == 26);
		return Token::SignedInt;
	case 3:
		assert(minor == 24);
		return Token::UTF8String;
	case 4:
		return Token::Array;
	case 5:
		return Token::Map;
	case 6:
		error("Unexpected major 6");
	case 7:
		switch (minor) {
		case 20:
			return Token::False;
		case 21:
			return Token::True;
		case 22:
			return Token::Null;
		case 23:
			return Token::Undefined;
		case 26:
			return Token::Float;
		case 31:
			return Token::Break;
		default:
			error("Unexpected minor %u for major 7", minor);
		}
	}
	return Token::Invalid;
}

void ReadStream::expect(Token expected) {
	Token t = nextToken();
	if (t != expected) {
		error("Expected: %u actual: %u", expected, t);
	}
}

uint32 ReadStream::readUInt() {
	expect(Token::UnsignedInt);
	return _stream.readUint32BE();
}

Common::String ReadStream::readString() {
	auto len = _stream.readByte();
	Common::String ret{len, 0};
	_stream.read(&ret[0], len);
	return ret;
}

} // end of namespace CBOR
} // end of namespace Kq8