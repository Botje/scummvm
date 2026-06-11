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

#include "kq8/kq_file.h"
#include "common/archive.h"
#include "common/str.h"
#include "common/stream.h"

namespace Kq8 {

bool KQFile::loadFromStream(Common::SeekableReadStream &stream) {
	Section section;
	KeyValue kv;
	Common::String comment;
	int lineno = 0;
	section.name = "";

	// TODO: Detect if a section occurs multiple times (or likewise, if
	// a key occurs multiple times inside one section).

	while (!stream.eos() && !stream.err()) {
		lineno++;

		// Read a line
		Common::String line = stream.readLine();

		line.trim();

		if (line.size() == 0) {
			// Do nothing
		} else if (line[0] == '#' || line[0] == ';' || line.hasPrefix("//") || line[0] == '"') {
			// Accumulate comments here. Once we encounter either the start
			// of a new section, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity. The semicolon and
			// C++-style comments are used for Living Books games in Mohawk.
			comment += line;
			comment += "\n";
		} else if (line[0] == '(') {
			// HACK: The following is a hack added by Kirben to support the
			// "map.ini" used in the HE SCUMM game "SPY Fox in Hold the Mustard".
			//
			// It would be nice if this hack could be restricted to that game,
			// but the current design of this class doesn't allow to do that
			// in a nice fashion (a "isMustard" parameter is *not* a nice
			// solution).
			comment += line;
			comment += "\n";
		} else if (line[0] == '[') {
			// It's a new section which begins here.
			const char *p = line.c_str() + 1;
			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// periods, dashes and underscores). Mohawk Living Books games
			// can have periods in their section names.
			// WinAGI games can have colons in their section names.
			while (*p && (Common::isAlnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == ' ' || *p == ':'))
				p++;

			if (*p == '\0') {
				warning("INIFile::loadFromStream: missing ] in line %d", lineno);
				return false;
			} else if (*p != ']') {
				warning("INIFile::loadFromStream: Invalid character '%c' occurred in section name in line %d", *p, lineno);
				return false;
			}

			// Previous section is finished now, store it.
			if (!section.name.empty())
				_sections.push_back(section);

			section.name = Common::String(line.c_str() + 1, p);
			section.keys.clear();
			section.comment = comment;
			comment.clear();

		} else {
			// This line should be a line with a 'key=value' pair, or an empty one.

			// If no section has been set, this config file is invalid!
			if (section.name.empty()) {
				warning("INIFile::loadFromStream: Key/value pair found outside a section in line %d", lineno);
				return false;
			}

			// Split string at '=' into 'key' and 'value'. First, find the "=" delimeter.
			const char *p = strchr(line.c_str(), '=');
			if (!p) {
				kv.key = line;
				kv.value.clear();
			} else {
				// Extract the key/value pair
				kv.key = Common::String(line.c_str(), p);
				kv.value = Common::String(p + 1);
			}

			// Trim of spaces
			kv.key.trim();
			kv.value.trim();

			// Store comment
			kv.comment = comment;
			comment.clear();

			section.keys.push_back(kv);
		}
	}

	// Save last section
	if (!section.name.empty())
		_sections.push_back(section);

	return (!stream.err() || stream.eos());
}

bool KQFile::loadFromFile(const Common::String &file) {
	auto stream = Common::ScopedPtr<Common::SeekableReadStream>{SearchMan.createReadStreamForMember(Common::Path{file})};
	if (!stream) {
		warning("Could not open file %s", file.c_str());
		return false;
	}

	_file = file;
	return loadFromStream(*stream);
}

const KQFile::Section *KQFile::getSection(const Common::String &name) const {
	for (auto &section : _sections) {
		if (section.name.equalsIgnoreCase(name)) {
			return &section;
		}
	}
	return nullptr;
}
} // namespace Kq8