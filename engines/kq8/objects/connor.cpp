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

#include "kq8/objects/connor.h"

#include "kq8/kq8.h"

namespace Kq8 {

Object *Connor::factory(const KQFile &f) {
	return new Connor(f);
}
Connor::Connor(const KQFile &f) : Object{f, false} {
}

void Connor::startSpecialAnimation(const Common::String &animListName, const Common::Array<Common::String> &loops) {
	auto *loopList = g_engine->graphicsManager().loadAnimationLoopList(animListName);
	debug("%s: starting animation list", name().c_str());
	_specialAnimation.reset(new SpecialAnimation{loopList, loops});
}


void Connor::update(float dt) {
	if (_specialAnimation) {
		bool animationFinished = _specialAnimation->advanceAnimation(dt);

		if (animationFinished) {
			debug("%s: advancing animation list (%d remain)", name().c_str(), _specialAnimation->_loopNames.size() - 1);
			bool finished = _specialAnimation->advanceLoop();
			if (finished) {
				debug("%s: finished animation list", name().c_str());
				_specialAnimation.reset();
			}
		}
	}
}

void Connor::draw() {
	if (_specialAnimation) {
		Math::Matrix4 objectTransform = getTransform();
		auto *shape = _specialAnimation->_currentLoop->_shape;
		int sequence = shape->_loops[0].sequenceIndex + _specialAnimation->_frame;
		g_engine->gfx().drawShape(shape, sequence, objectTransform);
	}
}

enum { kAnimationFPS = 15 };

bool Connor::SpecialAnimation::advanceLoop() {
	_loopNames.remove_at(0);
	if (_loopNames.empty())
		return true;

	_currentLoop = _loopList->getLoop(_loopNames[0]);
	_time = fmod(_time, 1.f / kAnimationFPS);
	_frame = 0;
	return false;
}

bool Connor::SpecialAnimation::advanceAnimation(float dt) {
	_time += dt;
	_frame = floor(_time * kAnimationFPS);
	return _frame >= _currentLoop->_frames;
}

} // namespace Kq8