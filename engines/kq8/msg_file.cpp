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
#include "common/str.h"
#include "common/stream.h"

#include "kq8/msg_file.h"

#include "common/stablemap.h"

namespace Kq8 {
MsgFile::MsgFile(const Common::String &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path(path)));

	stream->skip(10);
	auto count = stream->readUint16LE();

	Common::HashMap<uint16, Message *> offsetToMessage;

	for (int i = 0; i < count; ++i) {
		Message m;
		uint16 textOffset;
		stream->readMultipleLE(m._noun, m._verb, m._case, m._sequence, m._talker, textOffset);
		stream->skip(4);
		auto &inserted = _messages[m._talker][m.NVCS()] = m;
		offsetToMessage[textOffset + 2] = &inserted;
	}

	for (int i = 0; i < count; ++i) {
		uint16 offset = stream->pos();
		auto str = stream->readString();
		auto *msg = offsetToMessage.getValOrDefault(offset);
		if (msg) {
			msg->_str = Common::move(str);
		} else {
			warning("%s: Unreferenced message at offset %d", path.c_str(), offset);
		}
	}
}

const Common::String MsgFile::getMessage(uint8 talker, uint8 noun, uint8 verb, uint8 kase, uint8 sequence) {
	if (!_messages.contains(talker))
		return "";
	auto it = _messages[talker].find(MKTAG(noun, verb, kase, sequence));
	if (it != _messages[talker].end())
		return it->second._str;
	return "";
}
} // namespace Kq8