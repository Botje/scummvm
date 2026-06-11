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

void AnimObject::saveToStream(CBOR::WriteStream &out) const {
	using CBOR::Token;
	Object::saveToStream(out);
	if (_animation) {
		out << "animation" << Token::Array;
		out << CBOR::WithArgument{Token::Array, _animation->_loopList.size()};
		for (auto *loop : _animation->_loopList) {
			out << loop->_name;
		}
		out << _animation->_repeat << _animation->_time << _animation->_frame;
		out << Common::distance(_animation->currentLoop()->_cue.begin(), _animation->_nextCue);
		out << Token::Break;
	}
}

Object::AttributeSetters AnimObject::attributesToLoad() const {
	using CBOR::Token;

	auto ret = Object::attributesToLoad();
#define OBJ static_cast<AnimObject *>(it)
	ret["animation"] = [](Object *it, CBOR::ReadStream &in) {
		in.expect(Token::Array);
		in.expect(Token::Array);
		Common::Array<Common::String> loopNames(in.argument());
		for (auto &loopName : loopNames) {
			loopName = in.readString();
		}
		bool repeat = in.readBool();
		float time = in.readFloat();
		int frame = in.readSInt();
		uint32 cue = in.readUInt();
		in.expect(Token::Break);

		Common::SharedPtr<AnimationSequence> anim{new AnimationSequence{OBJ->_animationLoopList.get(), OBJ, loopNames, repeat}};
		anim->_time = time;
		anim->_frame = frame;
		anim->_nextCue = anim->currentLoop()->_cue.begin() + cue;
		OBJ->_animation = Common::move(anim);
	};
#undef OBJ
	return ret;
}

} // namespace Kq8