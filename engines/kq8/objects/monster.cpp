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

#include "kq8/objects/monster.h"

#include "audio/mixer.h"
#include "kq8/kq8.h"

namespace Kq8 {

using namespace INIHelpers;

Object *Monster::factory(const KQFile &f) {
	return new Monster(f);
}
Monster::Monster(const KQFile &f) : AnimObject{f} {
	auto &section = f.getSections().front();
	auto *animFileKey = section.getKey("animFile");
	if (animFileKey) {
		loadAnimLoopFromFile(animFileKey->value);
	}
	_alarmRadius = get<float>(section, "alarmRadius");
	_chaseRadius = get<float>(section, "chaseRadius");
	_homeRadius = get<float>(section, "homeRadius");
}

void Monster::addToInventory(const ItemType *itemType, uint16 quantity) {
	_inventory[itemType] += quantity;
}

void Monster::removeFromInventory(ItemType *itemType, uint16 quantity) {
	if (_inventory[itemType] < quantity)
		_inventory.erase(itemType);
	else
		_inventory[itemType] -= quantity;
}

template<class T>
static char base36Digit(T &x) {
	int c = x % 36;
	char ret = c < 10 ? c + '0' : c - 10 + 'A';
	x /= 36;
	return ret;
}

Monster::SpeakingState::SpeakingState(const uint16 catalog, const uint8 talker, const uint8 noun, const uint8 verb, const uint8 kase, const uint8 startSeq, const uint8 endSeq)
	: _catalog{catalog},
	  _talker{talker},
	  _noun{noun},
	  _verb{verb},
	  _kase{kase},
	  _startSeq{startSeq},
	  _endSeq{endSeq},
	  _curSeq{startSeq} {
	start();
}

void Monster::SpeakingState::start() {
	if (_curSeq > _endSeq) {
		return;
	}
	_msg = g_engine->graphicsManager().getMessage(_catalog, _talker, _noun, _verb, _kase, _curSeq);
	debugC(kDebugSpeech, "%s: %s", "", _msg.c_str());

	// TODO: lipsync file is the same but starts with S
	Common::String fileName{"AFFFNNVV.CCS"};
	char *p = fileName.end() - 1;
	*p-- = base36Digit(_curSeq);
	*p-- = base36Digit(_kase);
	*p-- = base36Digit(_kase);
	*p-- = '.';
	*p-- = base36Digit(_verb);
	*p-- = base36Digit(_verb);
	*p-- = base36Digit(_noun);
	*p-- = base36Digit(_noun);
	*p-- = base36Digit(_catalog);
	*p-- = base36Digit(_catalog);
	*p-- = base36Digit(_catalog);
	_handle = g_engine->playSound(fileName, Audio::Mixer::kSpeechSoundType);
}

bool Monster::SpeakingState::update() {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		return false;
	_curSeq++;
	if (_curSeq > _endSeq)
		return true;

	start();
	return false;
}

void Monster::speak(uint16 catalog, uint8 noun, uint8 verb, uint8 kase, uint8 startSeq, uint8 endSeq) {
	auto it = Common::find_if(_speaking.begin(), _speaking.end(), [=](const SpeakingState &ss) {
		return ss._catalog == catalog &&
			   ss._noun == noun &&
			   ss._verb == verb &&
			   ss._kase == kase &&
			   ss._startSeq == startSeq &&
			   ss._endSeq == endSeq;
	});

	if (it != _speaking.end()) {
		return;
	}

	_speaking.emplace_back(catalog, 4, noun, verb, kase, startSeq, endSeq);
}

void Monster::update(float dt) {
	AnimObject::update(dt);
	if (!_speaking.empty()) {
		bool done = _speaking.front().update();
		if (done)
			_speaking.pop_front();
	}
}

void Monster::saveToStream(CBOR::WriteStream &out) const {
	using namespace CBOR;
	AnimObject::saveToStream(out);
	out << "speaking" << WithArgument{Token::Array, _speaking.size()};
	for (const auto &s : _speaking) {
		out << s;
	}
	out << "inventory" << WithArgument{Token::Map, _inventory.size()};
	for (const auto &i : _inventory) {
		out << i._key->_idName << uint32(i._value);
	}
}
Object::AttributeSetters Monster::attributesToLoad() const {
	using namespace CBOR;
	auto ret = AnimObject::attributesToLoad();
#define OBJ static_cast<Monster *>(it)
	ret["speaking"] = [](Object *it, CBOR::ReadStream &in) {
		in.expect(Token::Array);
		OBJ->_speaking.clear();
		auto numSpeaking = in.argument();
		while (numSpeaking--) {
			in.expect(Token::Array);
			assert(in.argument() == 8);
			OBJ->_speaking.emplace_back();
			in >> OBJ->_speaking.back();
		}
	};
	ret["inventory"] = [](Object *it, CBOR::ReadStream &in) {
		OBJ->_inventory.clear();
		in.expect(Token::Map);
		auto numInventory = in.argument();
		while (numInventory--) {
			auto name = in.readString();
			auto quantity = in.readUInt();
			OBJ->_inventory.setVal(g_engine->reference().itemType(name), quantity);
		}
	};
#undef OBJ
	return ret;
}

CBOR::WriteStream &operator<<(CBOR::WriteStream &out, const Monster::SpeakingState &s) {
	using namespace CBOR;
	out << WithArgument{Token::Array, 8};
	out << s._catalog << s._talker << s._noun << s._verb << s._kase << s._startSeq << s._endSeq << s._curSeq;
	return out;
}

CBOR::ReadStream &operator>>(CBOR::ReadStream &in, Monster::SpeakingState &s) {
	using namespace CBOR;
	s._catalog = in.readUInt();
	s._talker = in.readUInt();
	s._noun = in.readUInt();
	s._verb = in.readUInt();
	s._kase = in.readUInt();
	s._startSeq = in.readUInt();
	s._endSeq = in.readUInt();
	s._curSeq = in.readUInt();
	return in;
}

} // namespace Kq8