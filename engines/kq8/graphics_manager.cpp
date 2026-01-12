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

#include "kq8/bitmap.h"
#include "kq8/font.h"
#include "kq8/graphics_manager.h"

#include "kq8.h"
#include "kq8/palette.h"
#include "pixel_formats.h"
#include "shape.h"

namespace Kq8 {
Graphics::Palette *GraphicsManager::getPalette(const Common::String &p) {
	if (_palettes.contains(p)) {
		return _palettes[p];
	}

	auto new_palette = Palette::loadPalette(Common::Path{p});
	if (new_palette) {
		_palettes[p] = new_palette;
		return _palettes[p];
	} else {
		return nullptr;
	}
}

GraphicsManager::~GraphicsManager() {
	for (auto &node : _palettes) {
		delete node._value;
	}
}

Font *GraphicsManager::loadFont(const Common::String &name, const Graphics::Palette *palette) {
	if (!palette) {
		error("Cannot load font from null palette");
	}
	if (_fonts.contains(name)) {
		return _fonts[name];
	}

	Common::ScopedPtr<Kq8::Font> ptr;
	ptr.reset(Kq8::Font::loadFont(name, palette));
	if (!ptr) {
		return nullptr;
	}
	g_engine->gfx().loadFont(ptr.get());
	_fonts[name] = ptr.get();
	return ptr.release();
}

Bitmap *GraphicsManager::loadBitmap(const Common::String &name, const Graphics::Palette *palette, const BitmapPacking packing) {
	if (_bitmaps.contains(name)) {
		return _bitmaps[name];
	}

	Common::ScopedPtr<Kq8::Bitmap> ptr;
	ptr.reset(Kq8::Bitmap::loadBitmap(name, palette));
	if (!ptr) {
		if (name.empty()) {
			auto *surface = new Graphics::Surface{};
			surface->create(1, 1, PixelFormats::getRGBPixelFormat());
			surface->setPixel(0, 0, 0xffff0000);
			ptr.reset(new Bitmap(surface));
		} else {
			return nullptr;
		}
	}

	if (packing == BitmapPacking::kPacked) {
		g_engine->gfx().loadBitmap(ptr.get());
	} else {
		g_engine->gfx().loadBitmapLoose(ptr.get());
	}
	_bitmaps[name] = ptr.get();
	return ptr.release();
}

void GraphicsManager::loadTerrain(Terrain *terrain) {
	g_engine->gfx().loadTerrain(terrain);
}
Shape *GraphicsManager::loadshape(const Common::String &name) {
	if (_shapes.contains(name)) {
		return _shapes[name];
	}

	Common::ScopedPtr<Kq8::Shape> ptr;
	ptr.reset(Kq8::Shape::loadShape(name));
	if (!ptr) {
		return nullptr;
	}
	g_engine->gfx().loadShape(ptr.get());
	_shapes[name] = ptr.get();
	return ptr.release();
}

void GraphicsManager::loadInterior(Interior *interior) {
	g_engine->gfx().loadInterior(interior);
}
AnimationLoopList *GraphicsManager::loadAnimationLoopList(const Common::String &name) {
	if (_animationLoops.contains(name)) {
		return _animationLoops[name];
	}

	Common::ScopedPtr<Kq8::AnimationLoopList> ptr;
	ptr.reset(AnimationLoopList::loadAnimationLoopList(name));
	if (!ptr) {
		return nullptr;
	}
	_animationLoops[name] = ptr.get();
	return ptr.release();
}

void GraphicsManager::drawBitmap(const Bitmap *bitmap, const Common::Rect &rect) {
	g_engine->gfx().drawBitmap(bitmap, rect);
}
void GraphicsManager::drawText(const Font *font, const Common::String &label, const Common::Rect &position) {
	g_engine->gfx().drawText(font, label, position);
}
void GraphicsManager::drawText(const Font *font, const Common::String &label, const Common::Point &position) {
	const auto bbox = font->boundingBox();
	g_engine->gfx().drawText(font, label, Common::Rect{position, static_cast<short>(label.size() * bbox.width()), bbox.height()});
}

const Common::String GraphicsManager::getMessage(int catalog, uint8 talker, uint8 noun, uint8 verb, uint8 kase, uint8 sequence) {
	if (!_msgFiles.contains(catalog)) {
		_msgFiles.setVal(catalog, MsgFile{Common::String::format("%d.msg", catalog)});
	}

	return _msgFiles[catalog].getMessage(talker, noun, verb, kase, sequence);
}

} // namespace Kq8