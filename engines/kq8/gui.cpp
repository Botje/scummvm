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
#include "kq8/read_helpers.h"

#include "common/archive.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/util.h"

namespace Kq8 {

static inline Common::Rect readRect(Common::SeekableReadStream *stream) {
	int16 left = stream->readUint32LE();
	int16 top = stream->readUint32LE();
	int16 right = stream->readUint32LE();
	int16 bottom = stream->readUint32LE();
	if (left > right || bottom > top)
		return Common::Rect{};
	else
		return Common::Rect{{left, top}, {right, bottom}};
}

Gui::Gui(const Common::String &filename, const Common::String &palette)
	: _palette{palette}, _filename{filename} {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{filename}));
	_rootDialog = readDialog(stream.get(), true);
}

Gui::~Gui() {
}

void Gui::prepare() {
	auto *palette = g_engine->graphicsManager().getPalette(_palette);
	prepareDialog(palette, _rootDialog);
}

void Gui::prepareDialog(Graphics::Palette *palette, Dialog &dialog) {
	if (dialog._tag == ControlType::kBitmapDialog) {
		dialog._gfxBitmap = g_engine->graphicsManager().loadBitmap(dialog._bitmap, palette);
	}
	for (auto &control : dialog._controls) {
		if (control._rect.isEmpty())
			continue;
		if (!control._bitmap.empty()) {
			control._gfxBitmap = g_engine->graphicsManager().loadBitmap(control._bitmap, palette);
		}
		if (!control._font.empty()) {
			control._gfxFont = g_engine->graphicsManager().loadFont(control._font, palette);
		}
	}

	for (auto &d : dialog._dialogs) {
		prepareDialog(palette, d);
	}
}

void Gui::draw() {
	drawDialog(_rootDialog);
}

void Gui::drawDialog(const Dialog &dialog) {
	if (dialog._tag == ControlType::kBitmapDialog) {
		g_engine->graphicsManager().drawBitmap(dialog._gfxBitmap, dialog._rect);
	}

	for (const auto &control : dialog._controls) {
		if (control._rect.isEmpty())
			continue;

		if (!control._bitmap.empty()) {
			g_engine->graphicsManager().drawBitmap(control._gfxBitmap, control._rect);
		}
		if (!control._font.empty()) {
			g_engine->graphicsManager().drawText(control._gfxFont, control._label, control._rect);
		}
	}

	for (auto &d : dialog._dialogs) {
		drawDialog(d);
	}
}

Gui::Dialog Gui::readDialog(Common::SeekableReadStream *stream, bool topLevel) {
	Dialog dialog;
	auto tag = stream->readUint32BE();
	switch (tag) {
	case MKTAG('C', 'C', 'B', 'D'):
		dialog._tag = ControlType::kBitmapDialog;
		break;
	case MKTAG('U', 'I', 'D', 'L'):
		dialog._tag = ControlType::kGuiDialog;
		break;
	default:
		error("wrong tag %x for dialog '%s'", tag, _filename.c_str());
	}

	/* auto len = */ stream->readUint32LE();
	/* auto version = */ stream->readUint32LE();
	dialog._id = stream->readUint32LE();
	/* auto flags = */ stream->readUint32LE();
	dialog._rect = readRect(stream);

	auto numDialogs = stream->readUint32LE();
	auto numControls = stream->readUint32LE();

	for (int i = 0; i < numDialogs; i++) {
		auto d = readDialog(stream, false);
		dialog._dialogs.emplace_back(Common::move(d));
	}

	for (int i = 0; i < numControls; i++) {
		auto control = readControl(stream);
		dialog._controls.emplace_back(Common::move(control));
	}

	if (dialog._tag == ControlType::kBitmapDialog) {
		stream->skip(4 * sizeof(uint32));
		dialog._bitmap = g_engine->getGuiTag(stream->readSint32LE());
		stream->skip(3 * sizeof(uint32));
	}

	return dialog;
}

Gui::Control Gui::readControl(Common::SeekableReadStream *stream) {
	Control item;
	uint32 tag = stream->readUint32BE();
	/* auto len = */ stream->readUint32LE();
	/* auto version = */ stream->readUint32LE();
	item._id = stream->readUint32LE();
	/* auto flags = */ stream->readUint32LE();
	item._rect = readRect(stream);

	switch (tag) {
	default:
		warning("unknown tag %x, assigning text", tag); // fallthrough
	case MKTAG('U', 'I', 'T', 'X'):
		item._tag = ControlType::kText;
		item._font = g_engine->getGuiTag(stream->readSint32LE());
		/* auto justification = */ stream->readUint32LE();
		item._label = g_engine->getGuiTag(stream->readUint32LE());
		break;
	case MKTAG('U', 'I', 'B', 'M'):
		item._tag = ControlType::kBitmap;
		item._bitmap = g_engine->getGuiTag(stream->readSint32LE());
		/* auto modifier = */ stream->readUint32LE();
		/* auto attribute = */ stream->readUint32LE();
		/* auto reserved = */ stream->readUint32LE();
		break;
	case MKTAG('C', 'C', 'B', 'B'):
		item._tag = ControlType::kButton;
		item._font = g_engine->getGuiTag(stream->readSint32LE());
		/* auto justification = */ stream->readUint32LE();
		item._label = g_engine->getGuiTag(stream->readUint32LE());
		/* auto bitmapArrayTag = */ stream->readSint32LE();
		/* auto modifier = */ stream->readUint32LE();
		/* auto attribute = */ stream->readUint32LE();
		/* auto reserved = */ stream->readUint32LE();
		break;
	}

	return item;
}

} // namespace Kq8