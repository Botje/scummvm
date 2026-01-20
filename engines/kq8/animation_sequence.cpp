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

#include "kq8/animation_sequence.h"
#include "kq8/kq8.h"

namespace Kq8 {

enum { kAnimationFPS = 15 };

void AnimationSequence::draw(const Math::Matrix4 &objectTransform) {
	auto *shape = currentLoop()->_shape;
	int sequence = shape->_loops[0].sequenceIndex + _frame;
	g_engine->gfx().drawShape(_owner, shape, objectTransform, sequence);
}

bool AnimationSequence::advanceLoop() {
	if (_loopList.size() > 1 || !_repeat) {
		_loopList.remove_at(0);
		if (_loopList.empty())
			return true;
	}

	auto *loop = currentLoop();
	_nextCue = loop->_cue.begin();
	_frame = loop->_start * (loop->_frames - 1);
	_time = _frame / kAnimationFPS;
	return false;
}

void AnimationSequence::processCue(Common::String command) {
	if (command.hasPrefix("sendEvent")) {
		command.replace(0, strlen("sendEvent"), "sendEvent 0");
		auto pos = command.find("@");
		if (pos != command.npos) {
			command.replace(pos, 1, _owner->name());
		}
	}
	g_engine->queueScript("<inline>", {command});
}

bool AnimationSequence::advanceAnimation(float dt) {
	auto *loop = currentLoop();
	if (loop->_speed == 0) {
		return false;
	}

	_time += dt * loop->_speed;
	_frame = floor(_time * kAnimationFPS);

	auto fraction = float(_frame) / (loop->_frames - 1);
	while (_nextCue != loop->_cue.end() && _nextCue->_percentage <= fraction) {
		processCue(_nextCue->_command);
		_nextCue++;
	}
	return loop->_speed >= 0 ? fraction >= 1 : fraction <= 0;
}

} // namespace Kq8