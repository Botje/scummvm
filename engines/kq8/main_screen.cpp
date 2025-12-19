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

#include "kq8/kq8.h"
#include "kq8/main_screen.h"

#include "common/archive.h"
#include "common/rect.h"
#include "common/stream.h"

namespace Kq8 {
struct MainScreen::ScreenItem {
	uint32 tag;
	Common::Rect rect;
	uint32 id;
	Common::String font;
	Common::String label;
	Common::String icon;
};

MainScreen::MainScreen() {
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
		item.tag = stream->readUint32BE();
		size_t item_len = stream->readUint32LE();
		uint32 values[13] = {};
		for (size_t v = 0; v * 4 < MIN(item_len, sizeof(values)); v++) {
			values[v] = stream->readUint32LE();
		}

		item.id = values[1];
		int16 x1 = values[3];
		int16 y1 = values[4];
		int16 x2 = values[5];
		int16 y2 = values[6];
		item.font = g_engine->getGuiTag(values[7]);
		item.label = g_engine->getGuiTag(values[9]);
		item.icon = item.tag == MKTAG('C', 'C', 'B', 'B') ? g_engine->getGuiTag(values[10]) : "";

		if (item_len > sizeof(values)) {
			stream->skip(item_len - sizeof(values));
		}
		item.rect = Common::Rect{x1, y1, x2, y2};
		_items.emplace_back(item);
	}
}

MainScreen::~MainScreen() {
}

void MainScreen::draw() {
}
} // Kq8