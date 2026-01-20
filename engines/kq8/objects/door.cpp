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

#include "kq8/objects/door.h"

namespace Kq8 {

Object *Door::factory(const KQFile &f) {
	return new Door(f);
}

Door::Door(const KQFile &f) : AnimObject(f) {
}

void Door::sendEvent(const Common::String &eventType, const Script::Args &args) {
	if (eventType != "Door") {
		AnimObject::sendEvent(eventType, args);
		return;
	}
	switch (_state) {
	case State::Closing:
	case State::Opening:
		return;
	case State::Closed:
		startAnimation({"open", "opened"}, true);
		_state = State::Opening;
		break;
	case State::Open:
		startAnimation({"close", "closed"}, true);
		_state = State::Closing;
		break;
	}
}

void Door::update(float dt) {
	AnimObject::update(dt);
	if ((_state == State::Opening || _state == State::Closing) && _animation->size() == 1) {
		_state = _state == State::Opening ? State::Open : State::Closed;
	}
}

} // namespace Kq8