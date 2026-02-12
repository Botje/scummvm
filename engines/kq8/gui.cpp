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

#include "kq8/gui.h"
#include "kq8/bitmap.h"
#include "kq8/font.h"
#include "kq8/kq8.h"

#include "common/archive.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/util.h"

namespace Kq8 {
enum class ScreenItemType {
	kText,
	kBitmap,
	kButton,
};

struct Gui::ScreenItem {
	ScreenItemType tag;
	Common::Rect rect;
	uint32 id;
	Common::String font;
	Common::String label;
	Common::String bitmap;
	Font *gfxFont;
	Bitmap *gfxBitmap;
};

Gui::Gui(const Common::String &palette)
	: _palette{palette} {
	_items.emplace_back(ScreenItem{ScreenItemType::kBitmap, Common::Rect{640, 480}, 0, "", "", "main18.pbm", nullptr, nullptr});

	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember("mainmenu.gui"));
	auto tag = stream->readUint32BE();
	if (tag != MKTAG('C', 'C', 'B', 'D')) {
		error("wrong tag for mainmenu.gui");
	}

	/* auto len = */ stream->readUint32LE();
	stream->skip(20);
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();
	stream->skip(4);

	uint32 num_items = stream->readUint32LE();
	for (uint32 i = 0; i < num_items; i++) {
		ScreenItem item;
		uint32 item_tag = stream->readUint32BE();
		switch (item_tag) {
		default:
			warning("unknown tag %x, assigning text", item_tag); // fallthrough
		case MKTAG('U', 'I', 'T', 'X'):
			item.tag = ScreenItemType::kText;
			break;
		case MKTAG('U', 'I', 'B', 'M'):
			item.tag = ScreenItemType::kBitmap;
			break;
		case MKTAG('C', 'C', 'B', 'B'):
			item.tag = ScreenItemType::kButton;
			break;
		}
		size_t item_len = stream->readUint32LE();
		uint32 values[11] = {};
		for (size_t v = 0; v * 4 < MIN(item_len, sizeof(values)); v++) {
			values[v] = stream->readUint32LE();
		}

		item.id = values[1];
		int16 x1 = values[3];
		int16 y1 = values[4];
		int16 x2 = values[5];
		int16 y2 = values[6];
		item.rect = Common::Rect{x1, y1, x2, y2};

		switch (item.tag) {
		case ScreenItemType::kText:
			item.font = g_engine->getGuiTag(values[7]);
			item.label = g_engine->getGuiTag(values[9]);
			break;

		case ScreenItemType::kBitmap:
			item.bitmap = g_engine->getGuiTag(values[7]);
			break;

		case ScreenItemType::kButton:
			item.bitmap = g_engine->getGuiTag(values[10]);
			break;
		}

		if (item_len > sizeof(values)) {
			stream->skip(item_len - sizeof(values));
		}
		_items.emplace_back(item);
	}
}

Gui::~Gui() {
}
void Gui::prepare() {
	auto *palette = g_engine->graphicsManager().getPalette(_palette);

	for (auto &item : _items) {
		if (!item.bitmap.empty()) {
			item.gfxBitmap = g_engine->graphicsManager().loadBitmap(item.bitmap, palette);
		}
		if (!item.font.empty()) {
			item.gfxFont = g_engine->graphicsManager().loadFont(item.font, palette);
		}
	}
}

void Gui::draw() {
	for (auto &item : _items) {
		if (!item.bitmap.empty()) {
			g_engine->graphicsManager().drawBitmap(item.gfxBitmap, item.rect);
		}
		if (!item.font.empty()) {
			g_engine->graphicsManager().drawText(item.gfxFont, item.label, item.rect);
		}
	}
}
} // namespace Kq8