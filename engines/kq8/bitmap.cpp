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
#include "image/bmp.h"

#include "kq8/bitmap.h"
#include "kq8/pixel_formats.h"

namespace Kq8 {

Graphics::Surface *Bitmap::parseBitmap(Common::SeekableReadStream *stream) {
	auto tag = stream->readUint32BE();
	if (tag != MKTAG('P', 'B', 'M', 'P')) {
		warning("wrong tag for font, expected PBMP");
		return nullptr;
	}
	/* auto len = */ stream->readUint32LE();

	tag = stream->readUint32BE();
	if (tag != MKTAG('h', 'e', 'a', 'd')) {
		warning("wrong tag for font, expected head");
		return nullptr;
	}
	/* auto head_len = */ stream->readUint32LE();
	stream->skip(4);
	auto width = stream->readUint32LE();
	auto height = stream->readUint32LE();
	stream->skip(4 * 2);

	tag = stream->readUint32BE();
	if (tag != MKTAG('d', 'a', 't', 'a')) {
		warning("wrong tag for font, expected tag");
		return nullptr;
	}
	auto dataLen = stream->readUint32LE();
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> surface;
	surface.reset(new Graphics::Surface);
	byte *pixels = new byte[dataLen];
	stream->read(pixels, dataLen);
	int16 pitch = width;
	if (width % 4) {
		pitch += 4 - (width % 4);
	}
	surface->init(width, height, pitch, pixels, Graphics::PixelFormat::createFormatCLUT8());

	tag = stream->readUint32BE();
	if (tag != MKTAG('D', 'E', 'T', 'L')) {
		warning("wrong tag for font, expected tag");
		return nullptr;
	}
	auto detlLen = stream->readUint32LE();
	stream->skip(detlLen);
	return surface.release();
}

Bitmap *Bitmap::loadBitmap(const Common::String &path, const Graphics::Palette *palette) {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path(path)));
	if (!stream) {
		warning("Could not load bitmap '%s'", path.c_str());
		return nullptr;
	}

	auto tag = stream->readUint32BE();
	if (tag == MKTAG('P', 'B', 'M', 'A')) {
		warning("Trying to load animated bitmap %s. Taking first frame.", path.c_str());
		stream->skip(8);
		uint32 head_len = stream->readUint32LE();
		stream->skip(head_len);
	} else if ((tag >> 16) == MKTAG16('B', 'M')) {
		stream->seek(0);
		Image::BitmapDecoder decoder;
		decoder.loadStream(*stream);

		auto *surface = new Graphics::Surface;
		surface->copyFrom(*decoder.getSurface());
		if (surface->format.bpp() != 24)
			surface->convertToInPlace(PixelFormats::getRGBPixelFormat(), decoder.getPalette().data(), decoder.getPalette().size());
		return new Bitmap{surface};
	} else {
		stream->seek(0);
	}
	auto *surface = parseBitmap(stream.get());
	if (!surface) {
		warning("Could not parse bitmap '%s'", path.c_str());
		return nullptr;
	}

	surface->convertToInPlace(PixelFormats::getRGBPixelFormat(), palette->data(), palette->size());

	return new Bitmap{surface, path};
}
} // namespace Kq8