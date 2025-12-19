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

#ifndef KQ8_ANIMATION_LOOP_LIST_H
#define KQ8_ANIMATION_LOOP_LIST_H

#include "common/archive.h"
#include "common/str.h"

#include "kq8/kq_file.h"
#include "kq8/shape.h"

namespace Kq8 {

class AnimationLoopList {
public:
	struct Loop {
		struct Cue {
			float _percentage;
			Common::String _command;
		};
		Loop() = default;
		Loop(int frames, const Common::String &shapeName, const Common::String &name, Shape *shape, const Common::Array<Cue> &cues, const Common::Array<Common::String> &transitions)
			: _frames{frames}, _shapeName{shapeName}, _name{name}, _shape{shape}, _cue{cues}, _transitions{transitions} {}
		int _frames;
		Common::String _shapeName;
		Common::String _name;
		Shape *_shape;
		Common::Array<Cue> _cue;
		Common::Array<Common::String> _transitions;
	};

public:
	AnimationLoopList(const Common::String &path);
	static AnimationLoopList *loadAnimationLoopList(const Common::String &path);
	Loop *getLoop(const Common::String &loop);

private:
	static Loop loopFromSection(const KQFile::Section &section, const Common::String &name, int i);
	Common::HashMap<Common::String, Loop> _loops;
};

} // namespace Kq8

#endif // KQ8_ANIMATION_LOOP_LIST_H
