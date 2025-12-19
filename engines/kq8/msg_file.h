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

#ifndef KQ8_MSG_FILE_H
#define KQ8_MSG_FILE_H

#include "common/stablemap.h"
#include "common/str.h"

namespace Kq8 {

class MsgFile {
	using NVCS = uint32;
	using Talker = uint8;

public:
	struct Message {
		uint8 _noun;
		uint8 _verb;
		uint8 _case;
		uint8 _sequence;
		Talker _talker;
		Common::String _str;

		uint32 NVCS() const { return MKTAG(_noun, _verb, _case, _sequence); }
	};
	MsgFile() = default;
	MsgFile(const Common::String &path);
	const Common::String getMessage(uint8 talker, uint8 noun, uint8 verb, uint8 kase, uint8 sequence);

private:
	using TalkerMap = Common::StableMap<NVCS, Message>;
	Common::HashMap<Talker, TalkerMap> _messages;
};

} // namespace Kq8

#endif // KQ8_ MSG_FILE_H
