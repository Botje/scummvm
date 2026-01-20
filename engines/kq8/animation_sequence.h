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

#ifndef KQ8_ANIMATION_SEQUENCE_H
#define KQ8_ANIMATION_SEQUENCE_H

#include "common/array.h"
#include "common/str.h"
#include "kq8/animation_loop_list.h"
#include "kq8/objects/object.h"

namespace Kq8 {

struct AnimationSequence {
	AnimationSequence(AnimationLoopList *loopList, const Object *owner, const Common::Array<Common::String> &loops, bool repeat = false)
		: _owner{owner}, _repeat{repeat} {
		for (const auto &loopName : loops) {
			_loopList.push_back(loopList->getLoop(loopName));
		}
		auto *loop = currentLoop();
		_nextCue = loop->_cue.begin();
		_frame = loop->_start * (loop->_frames - 1);
	}

	const AnimationLoopList::Loop *currentLoop() const { return _loopList.empty() ? nullptr : _loopList.front(); }
	void draw(const Math::Matrix4 &objectTransform);
	int size() const { return _loopList.size(); }

	using CueIterator = decltype(AnimationLoopList::Loop::_cue)::const_iterator;
	Common::Array<AnimationLoopList::Loop *> _loopList;
	int _frame = 0;
	float _time = 0;
	CueIterator _nextCue;
	const Object *_owner;
	bool _repeat;

	bool advanceLoop();
	void processCue(Common::String command);
	bool advanceAnimation(float dt);
};

} // namespace Kq8

#endif // KQ8_ANIMATION_SEQUENCE_H
