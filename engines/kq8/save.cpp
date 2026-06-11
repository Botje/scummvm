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
#include "kq8/kq8.h"
#include "objects/camera.h"

namespace Kq8 {
/*
 * SAVE GAME FORMAT
 * The top-level items of a save game are an array with the following elements:
 * [
 *   version,
 *   environment,
 *   connor,
 *   camera,
 *   { world_name => bytestring }
 * ]
 * Worlds other than the current one are frozen and only loaded when the world is re-entered.
 */
Common::Error Kq8Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	using namespace CBOR;
	WriteStream out{*stream};
	out << Token::Array << 1; // version
	out << WithArgument{Token::Map, _environment.size()};
	for (const auto &pair : _environment) {
		out << pair._key << pair._value;
	}

	// Connor and the camera
	out << Token::Map << *connor() << Token::Break;
	out << Token::Map << *camera() << Token::Break;

	out << Token::Map << world()->name() << *world() << Token::Break;
	out << Token::Break;

	return Common::kNoError;
}

Common::Error Kq8Engine::loadGameStream(Common::SeekableReadStream *stream) {
	using namespace CBOR;
	ReadStream in{*stream};
	in.expect(Token::Array);
	auto version = in.readUInt();
	assert(version == 1);

	_environment.clear();
	in.expect(Token::Map);
	for (int kvs = in.argument(); kvs >= 0; kvs--) {
		auto key = in.readString();
		auto value = in.readString();
		_environment[key] = Common::move(value);
	}
	_connor.reset(static_cast<Connor *>(_objectFactory.restoreFromSave(in)));
	_camera.reset(new Camera);
	_camera->loadAttributesFromStream(in);

	_worldStates.clear();
	in.expect(Token::Map);
	while (in.peekToken() != Token::Break) {
		auto name = in.readString();
		_worldStates[name] = in.readByteString();
	}
	in.expect(Token::Break);

	return Common::kNoError;
}

} // End of namespace Kq8