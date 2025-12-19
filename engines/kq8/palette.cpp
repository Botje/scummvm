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

#include "common/ptr.h"

#include "kq8/palette.h"

#include "common/archive.h"
#include "common/stream.h"

namespace Kq8 {
Graphics::Palette *Palette::loadPalette(const Common::Path &path) {
	Common::ScopedPtr<Graphics::Palette> ret(new Graphics::Palette(256));
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(path));
	if (!stream) {
		warning("Could not open palette file '%s'", path.toString().c_str());
		return nullptr;
	}

	auto tag = stream->readUint32BE();
	if (tag != MKTAG('P', 'P', 'A', 'L')) {
		error("Not a palette file '%s'", path.toString().c_str());
	}

	/* auto len = */ stream->skip(4);
	while (true) {
		auto chunk_tag = stream->readUint32BE();
		if (stream->eos()) {
			break;
		}
		auto chunk_size = stream->readUint32LE();
		if (chunk_tag != MKTAG('d', 'a', 't', 'a')) {
			stream->skip(chunk_size);
		} else {
			if (chunk_size != 1024) {
				error("Data chunk for palette '%s' is wrong size! Expected 1024 got %d", path.toString().c_str(), chunk_size);
			}
			byte r, g, b, unused;
			for (int i = 0; i < 256; i++) {
				stream->readMultipleBE(r, g, b, unused);
				ret->set(i, r, g, b);
			}
			return ret.release();
		}
	}

	return ret.release();
}
} // namespace Kq8