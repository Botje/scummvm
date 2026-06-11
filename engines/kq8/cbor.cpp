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

enum {
	kIndefiniteLength = ~0ULL,
};

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
		return writeHead(2, s._size);
	case Token::UTF8String:
		return writeHead(3, s._size);
	case Token::Array:
		return writeHead(4, s._size);
	case Token::Map:
		return writeHead(5, s._size);
	default:
		error("Attempt to write CBOR::WithArgument with wrong major type %u", s._token);
	}
	return *this;
}

WriteStream &WriteStream::operator<<(uint32 i) {
	return writeHead(0, i);
}

WriteStream &WriteStream::operator<<(int32 i) {
	if (i >= 0) {
		return *this << uint32(i);
	} else {
		return writeHead(1, abs(i) - 1);
	}
}

WriteStream &WriteStream::operator<<(float f) {
	_stream.writeByte(7 << 5 | 26);
	_stream.writeFloatBE(f);
	return *this;
}

WriteStream &WriteStream::operator<<(const char *s) {
	size_t sl = strlen(s);
	writeHead(3, sl);
	_stream.write(s, sl);
	return *this;
}

WriteStream &WriteStream::operator<<(const Common::String &s) {
	writeHead(3, s.size());
	_stream.writeString(s);
	return *this;
}

WriteStream &WriteStream::operator<<(const Math::Vector3d &v) {
	return *this << WithArgument{Token::Array, 3} << v.x() << v.y() << v.z();
}

WriteStream &WriteStream::operator<<(const ByteString &bs) {
	*this << WithArgument{Token::ByteString, uint32(bs._bytes.size())};
	_stream.write(bs._bytes.data(), bs._bytes.size());
	return *this;
}

WriteStream &WriteStream::operator<<(bool b) {
	return writeHead(7, b ? 21 : 20);
}

WriteStream &WriteStream::writeHead(int major, uint64 argument) {
	if (argument < 24) {
		_stream.writeByte(major << 5 | byte(argument));
	} else if (argument < (1 << 8)) {
		_stream.writeByte(major << 5 | 24);
		_stream.writeByte(argument);
	} else if (argument < (1 << 16)) {
		_stream.writeByte(major << 5 | 25);
		_stream.writeUint16BE(argument);
	} else if (argument < (1ULL << 32)) {
		_stream.writeByte(major << 5 | 26);
		_stream.writeUint32BE(argument);
	} else {
		_stream.writeByte(major << 5 | 27);
		_stream.writeUint64BE(argument);
	}
	return *this;
}

Token ReadStream::peekToken() {
	if (_peeked) {
		return _tokenType;
	}
	Token ret = nextToken();
	_peeked = true;
	return ret;
}

Token ReadStream::nextToken() {
	if (_peeked) {
		_peeked = false;
		return _tokenType;
	}

	uint8 b = _stream.readByte();
	if (_stream.eos()) {
		_argument = 0;
		return _tokenType = Token::Eof;
	}

	uint8 major = b >> 5u;
	uint8 minor = b & 0x1fu;

	auto resolveArgument = [=](uint8 minor) -> uint64 {
		if (minor < 24)
			return minor;
		switch (minor) {
		case 24:
			return _stream.readByte();
		case 25:
			return _stream.readUint16BE();
		case 26:
			return _stream.readUint32BE();
		case 27:
			return _stream.readUint64BE();
		case 31:
			return kIndefiniteLength;
		default:
			error("Unexpected argument %u", minor);
		}
	};

	switch (major) {
	case 0:
		_argument = resolveArgument(minor);
		return _tokenType = Token::UnsignedInt;
	case 1:
		_argument = resolveArgument(minor);
		return _tokenType = Token::SignedInt;
	case 2:
		_argument = resolveArgument(minor);
		return _tokenType = Token::ByteString;
	case 3:
		_argument = resolveArgument(minor);
		return _tokenType = Token::UTF8String;
	case 4:
		_argument = resolveArgument(minor);
		return _tokenType = Token::Array;
	case 5:
		_argument = resolveArgument(minor);
		return _tokenType = Token::Map;
	case 6:
		_argument = resolveArgument(minor);
		return _tokenType = Token::Tag;
	case 7:
		_argument = 0;
		switch (minor) {
		case 20:
			return _tokenType = Token::False;
		case 21:
			return _tokenType = Token::True;
		case 22:
			return _tokenType = Token::Null;
		case 23:
			return _tokenType = Token::Undefined;
		case 25:
		case 26:
		case 27:
			_argument = minor;
			return _tokenType = Token::Float;
		case 31:
			return _tokenType = Token::Break;
		default:
			error("Unexpected minor %u for major 7", minor);
		}
	default:
		_argument = 0;
		return _tokenType = Token::Invalid;
	}
}

void ReadStream::expect(Token expected) {
	Token t = nextToken();
	if (t != expected) {
		error("Expected: %u actual: %u", expected, t);
	}
}

uint32 ReadStream::readUInt() {
	expect(Token::UnsignedInt);
	return argument();
}

Common::String ReadStream::readString() {
	expect(Token::UTF8String);
	auto len = argument();
	return _stream.readString(0, len);
}

Common::Array<byte> ReadStream::readByteString() {
	expect(Token::ByteString);
	auto size = argument();
	Common::Array<byte> ret;
	ret.resize(size);
	_stream.read(ret.data(), size);
	return ret;
}

Math::Vector3d ReadStream::readVector3d() {
	Math::Vector3d ret;
	expect(Token::Array);
	ret.x() = readFloat();
	ret.y() = readFloat();
	ret.z() = readFloat();
	return ret;
}

float ReadStream::readFloat() {
	expect(Token::Float);
	assert(argument() == 26);
	return _stream.readFloatBE();
}

int32 ReadStream::readSInt() {
	Token t = nextToken();
	switch (t) {
	case Token::SignedInt:
		return -1 - int32(argument());
	case Token::UnsignedInt:
		return int32(argument());
	default:
		error("Unexpected token %u for signed int", t);
	}
}
bool ReadStream::readBool() {
	Token t = nextToken();
	switch (t) {
	case Token::True:
		return true;
	case Token::False:
		return false;
	default:
		error("Unexpected token %u for bool", t);
	}
}

} // end of namespace CBOR
} // end of namespace Kq8