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

#ifndef KQ8_KQ_FILE_H
#define KQ8_KQ_FILE_H

#include "common/formats/ini-file.h"

namespace Kq8 {

class KQFile {
public:
	using Section = Common::INIFile::Section;
	using SectionList = Common::INIFile::SectionList;
	using KeyValue = Common::INIFile::KeyValue;
	bool loadFromStream(Common::SeekableReadStream &stream);
	SectionList &getSections() { return _sections; }
	const SectionList &getSections() const { return _sections; }

private:
	SectionList _sections;
};

namespace INIHelpers {

template<class T>
T extractFromValue(const Common::String &);

template<>
inline Common::String extractFromValue(const Common::String &s) {
	return s;
}

template<>
inline float extractFromValue<float>(const Common::String &s) {
	return strtof(s.c_str(), nullptr);
}

template<>
inline uint8 extractFromValue<uint8>(const Common::String &s) {
	return strtol(s.c_str(), nullptr, 10);
}

template<>
inline uint16 extractFromValue<uint16>(const Common::String &s) {
	return strtol(s.c_str(), nullptr, 10);
}

template<>
inline int extractFromValue<int>(const Common::String &s) {
	return strtol(s.c_str(), nullptr, 10);
}

template<>
inline long extractFromValue<long>(const Common::String &s) {
	return strtol(s.c_str(), nullptr, 10);
}

template<class T>
T get(const KQFile::Section &sec, const Common::String &key, const T &def = T{}) {
	auto *kv = sec.getKey(key);
	if (!kv)
		return def;
	return extractFromValue<T>(kv->value);
}

template<class T>
Common::Array<T> getArray(const KQFile::Section &sec, const Common::String &countKey, const Common::String &prefix) {
	auto count = get<int>(sec, countKey);
	Common::Array<T> array;
	array.reserve(count);
	for (int i = 0; i < count; i++) {
		auto key = Common::String::format("%s%d", prefix.c_str(), i);
		array.push_back(get<T>(sec, key));
	}
	return array;
}

} // namespace INIHelpers

} // namespace Kq8

#endif // KQ_FILE_H
