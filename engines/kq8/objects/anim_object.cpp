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

#include "kq8/objects/anim_object.h"

#include "kq8/animation_loop_list.h"
#include "kq8/animation_sequence.h"

namespace Kq8 {
Object *AnimObject::factory(const KQFile &f) {
	return new AnimObject(f);
}

void AnimObject::startAnimation(const Common::Array<Common::String> &animations, bool repeat) {
	_animation.reset(new AnimationSequence{_animationLoopList.get(), this, animations, repeat});
}

void AnimObject::loadAnimLoopFromFile(const Common::String &path) {
	_animationLoopList.reset(AnimationLoopList::loadAnimationLoopList(path));
	startAnimation({"stop"}, true);
}

AnimObject::AnimObject(const KQFile &f) : Object{f} {
	auto &section = f.getSections().front();

	auto *animLoopsKey = section.getKey("animLoops");
	if (animLoopsKey) {
		_animationLoopList.reset(new AnimationLoopList{f, animLoopsKey->value});

		auto *currAnimKey = section.getKey("currAnimName");
		if (currAnimKey) {
			startAnimation({currAnimKey->value}, true);
		}
	}
}

void AnimObject::draw() {
	if (_animation) {
		_animation->draw(getTransform());
	}
}

void AnimObject::update(float dt) {
	Object::update(dt);
	if (_animation) {
		bool done = _animation->advanceAnimation(dt);
		if (done) {
			_animation->advanceLoop();
		}
	}
}

} // namespace Kq8