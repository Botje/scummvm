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

#include "kq8/objects/terrain.h"

#include "common/archive.h"
#include "common/stream.h"
#include "image/bmp.h"

namespace Kq8 {

Object *Terrain::factory(const KQFile &f) {
	return new Terrain(f);
}
Terrain::Terrain(const KQFile &f) {
	auto &section = f.getSections().front();
	auto materialBMP = section.getKey("materialBMP")->value;
	Common::ScopedPtr<Common::SeekableReadStream> stream{SearchMan.createReadStreamForMember(Common::Path{
		materialBMP})};
	Image::BitmapDecoder decoder;
	decoder.loadStream(*stream);
	auto surface = decoder.getSurface();
}

} // namespace Kq8