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

#include "kq8/material_file.h"
#include "kq8/read_helpers.h"

namespace Kq8 {

Common::Pair<Common::Array<MaterialName>, Common::Array<uint8> > loadMaterialFile(const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{name}));
	stream->skip(26);
	auto version = stream->readUint32LE();
	stream->skip(4);
	auto num_materials = stream->readUint32LE();

	Common::Pair<Common::Array<Common::String>, Common::Array<uint8> > ret;
	Common::Array<Common::String> &materials = ret.first;
	Common::Array<uint8> &mapping = ret.second;

	while (num_materials--) {
		/* auto flags = */ stream->readUint16LE();
		stream->skip(14);
		auto mat = readBoundedString(stream.get(), version < 2 ? 16 : 32);

		auto materialIt = Common::find(materials.begin(), materials.end(), mat);
		if (materialIt != materials.end()) {
			mapping.push_back(Common::distance(materials.begin(), materialIt));
		} else {
			materials.push_back(mat);
			mapping.push_back(materials.size() - 1);
		}
	}
	return ret;
}

} // namespace Kq8