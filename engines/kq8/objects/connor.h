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

#ifndef KQ8_OBJECTS_CONNOR_H
#define KQ8_OBJECTS_CONNOR_H

#include "common/tokenizer.h"
#include "kq8/animation_loop_list.h"
#include "kq8/objects/object.h"

namespace Kq8 {

class Connor : public Object {
public:
	static Object *factory(const KQFile &f);
	Connor(const KQFile &f);
	void startSpecialAnimation(const Common::String &animListName, const Common::Array<Common::String> &loops);
	void update(float dt) override;
	void draw() override;

private:
	struct SpecialAnimation {
		SpecialAnimation(AnimationLoopList *loopList, const Common::String &owner, const Common::Array<Common::String> &loops)
			: _loopList{loopList},
			  _owner{owner},
			  _loopNames{loops},
			  _currentLoop{loopList->getLoop(loops[0])} {
			_nextCue = _currentLoop->_cue.begin();
		}

		using CueIterator = decltype(AnimationLoopList::Loop::_cue)::const_iterator;
		AnimationLoopList *_loopList;
		Common::Array<Common::String> _loopNames;
		AnimationLoopList::Loop *_currentLoop;
		int _frame = 0;
		float _time = 0;
		CueIterator _nextCue;
		Common::String _owner;

		bool advanceLoop();
		bool advanceAnimation(float dt);
	};
	Common::ScopedPtr<SpecialAnimation> _specialAnimation;
};

} // namespace Kq8

#endif // KQ8_OBJECTS_CONNOR_H
