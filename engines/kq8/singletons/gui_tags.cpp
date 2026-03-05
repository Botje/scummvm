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
#include "common/stream.h"

#include "kq8/singletons/gui_tags.h"

// This file starts with [TAG] end ends with [END].
// The lines we care about are of the form
//    00000725 IDSTR_CANCEL '#35 Cancel'
Common::HashMap<int, Common::String> Kq8::Singleton::loadGuiTags() {
	Common::HashMap<int, Common::String> ret;
	auto stream = SearchMan.createReadStreamForMember("kqGuiTag.TTAG");
	if (!stream) {
		error("Could not load 'kqGuiTag.TTAG' file");
		return ret;
	}

	/* auto tag = */ stream->readLine();
	while (true) {
		auto line = stream->readLine();
		if (stream->eos() || line == "[END]") {
			break;
		}

		auto firstSpace = line.find(' ');
		auto firstQuote = line.find('\'');
		auto lastQuote = line.rfind('\'');
		if (firstSpace == Common::String::npos || firstQuote == Common::String::npos || lastQuote == Common::String::npos || firstQuote == lastQuote) {
			warning("Could not parse line %s", line.c_str());
		} else {
			auto key = line.substr(0, firstSpace).asUint64();
			auto value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
			ret[key] = Common::move(value);
		}
	}

	return ret;
}