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

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/tokenizer.h"

#include "kq8/animation_loop_list.h"

#include "kq8.h"
#include "kq8/kq_file.h"

namespace Kq8 {

using Loop = AnimationLoopList::Loop;

namespace INIHelpers {
template<>
inline Loop::Cue extractFromValue<Loop::Cue>(const Common::String &s) {
	char *rest;
	auto percentage = strtof(s.c_str(), &rest);

	auto command = Common::String{Common::ltrim(rest)};
	return Loop::Cue{percentage, command};
}
} // namespace INIHelpers

using namespace INIHelpers;

inline float to_f(const Common::String &s) {
	return strtof(s.c_str(), nullptr);
}

Loop AnimationLoopList::loopFromSection(const KQFile::Section &section, const Common::String &name, int i) {
	auto loop = get<Common::String>(section, "loop");
	Common::StringTokenizer tokens{loop};
	auto shape = tokens.nextToken();
	auto loopName = tokens.nextToken();
	auto start = to_f(tokens.nextToken());
	auto speed = to_f(tokens.nextToken());
	auto unk1 = to_f(tokens.nextToken());
	auto unk2 = to_f(tokens.nextToken());

	auto *shapeObj = g_engine->graphicsManager().loadshape(shape);
	auto frames = shapeObj->_loops[0].sequenceCount;
	auto cues = getArray<Loop::Cue>(section, "nCue", "cue");
	auto transitions = getArray<Common::String>(section, "nTransition", "transition");
	return Loop{frames, shape, name, shapeObj, cues, transitions, start, speed};
}

AnimationLoopList::AnimationLoopList(const KQFile &f, const Common::String &section) {

	auto *mainSection = f.getSection(section);
	if (!mainSection) {
		error("Missing section '%s'", section.c_str());
	}

	const auto movements = getArray<Common::String>(*mainSection, "nMovement", "Movement");
	auto nRequiredLoops = get<int>(*mainSection, "nRequiredLoops");
	auto nSpecificLoops = get<int>(*mainSection, "nSpecificLoops");
	auto nTransitionLoops = get<int>(*mainSection, "nTransitionLoops");

	for (int i = 0; i < movements.size(); i++) {
		const auto &name = movements[i];
		const auto sectionName = Common::String::format("movement%d", i);
		const auto *s = f.getSection(sectionName);
		if (!s) {
			error("Missing section '%s' for movement %s", sectionName.c_str(), name.c_str());
		}
		_loops[name] = loopFromSection(*s, name, i);
	}
}
AnimationLoopList *AnimationLoopList::loadAnimationLoopList(const Common::String &path) {
	Common::SharedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{path}));

	KQFile f;
	f.loadFromStream(*stream);
	return new AnimationLoopList(f, f.getSections().front().name);
}

Loop *AnimationLoopList::getLoop(const Common::String &loop) {
	if (_loops.contains(loop)) {
		return &_loops[loop];
	}
	return nullptr;
}

} // namespace Kq8