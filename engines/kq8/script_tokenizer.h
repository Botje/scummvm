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

#ifndef KQ8_SCRIPT_TOKENIZER_H
#define KQ8_SCRIPT_TOKENIZER_H

#include "common/array.h"
#include "common/str.h"

namespace Kq8 {
using Tokens = Common::Array<Common::String>;

struct ScriptTokenizer {
	explicit ScriptTokenizer(const Common::String &str) : str_(str), pos_(str.begin()) {
		reset();
	}
	const Common::String &str_;
	Common::String::const_iterator pos_;

	void advance() {
		while (pos_ != str_.end() && Common::isSpace(*pos_)) {
			pos_++;
		}
	}

	void reset() {
		pos_ = str_.begin();
		advance();
	}

	Tokens rest() {
		advance();
		Tokens ret;
		Common::String tok;
		while (!(tok = nextToken()).empty()) {
			ret.push_back(tok);
		}
		return ret;
	}

	Common::String nextToken() {
		advance();

		if (pos_ == str_.end() || *pos_ == '#') {
			pos_ = str_.end();
			return "";
		}

		if (*pos_ == '"' || *pos_ == '\'') {
			char startQuote = *pos_;
			pos_++;
			auto start = pos_;
			while (pos_ != str_.end() && *pos_ != startQuote) {
				pos_++;
			}

			// TODO: this ignores unbalanced quotes. TBD if this is a problem
			return Common::String(start, pos_++);
		} else {
			auto start = pos_;
			while (pos_ != str_.end() && !Common::isSpace(*pos_) && *pos_ != '#') {
				pos_++;
			}
			return Common::String(start, pos_);
		}
	}

	uint16 nextUint16() {
		auto tok = nextToken();
		uint16 ret;
		sscanf("%hu", tok.c_str(), &ret);
		return ret;
	}

	uint8 nextUint8() {
		auto tok = nextToken();
		uint8 ret;
		sscanf("%hhu", tok.c_str(), &ret);
		return ret;
	}

	int16 nextInt16() {
		auto tok = nextToken();
		int16 ret;
		sscanf("%hd", tok.c_str(), &ret);
		return ret;
	}
	float nextFloat() {
		auto tok = nextToken();
		float ret = strtof(tok.c_str(), nullptr);
		return ret;
	}
};

} // namespace Kq8
#endif // KQ8_SCRIPT_TOKENIZER_H