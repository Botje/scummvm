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
#include "kq8/objects/connor.h"
#include "kq8/read_helpers.h"

#include "common/archive.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/util.h"

namespace Kq8 {

enum knownIdTags {
	IDCTL_INVBART = 110,
	IDCTL_INVBAR_QUEST1 = 349,
	IDCTL_INVBAR_QUEST2 = 348,
	IDCTL_INVBAR_QUEST3 = 347,
	IDCTL_INVBAR_QUEST4 = 346,
	IDCTL_INVBAR_QUEST5 = 345,
	IDCTL_INVBAR_QUEST6 = 344,
	IDCTL_INVBAR_QUEST7 = 343,
	IDCTL_INVBAR_GOLD = 580,
	IDCTL_INVBAR_GOLDTEXT = 581,
	IDCTL_HEALTHBAR = 1,
	IDCTL_HEALTHBAR_1 = 268,
	IDCTL_HEALTHBAR_1TEXT = 232,
	IDCTL_HEALTHBAR_2 = 271,
	IDCTL_HEALTHBAR_2TEXT = 233,
	IDCTL_HEALTHBAR_3 = 272,
	IDCTL_HEALTHBAR_3TEXT = 234,
	IDCTL_HEALTHBAR_4 = 273,
	IDCTL_HEALTHBAR_4TEXT = 235,
	IDCTL_HEALTHBAR_5 = 274,
	IDCTL_HEALTHBAR_5TEXT = 236,
	IDCTL_HEALTHBAR_6 = 275,
	IDCTL_HEALTHBAR_6TEXT = 237,
	IDCTL_HEALTHBAR_7 = 276,
	IDCTL_HEALTHBAR_7TEXT = 238,
	IDCTL_HEALTHBAR_8 = 277,
	IDCTL_HEALTHBAR_8TEXT = 239,
	IDBMP_1EMPTY = 339,
	IDBMP_2GOLD3 = 525,
	IDCTL_METERS = 248,
	IDCTL_METERS_EXP = 291,
	IDCTL_METERS_LEVEL = 769,
	IDCTL_METERS_HEALTH = 289,
};

static inline Common::Rect readRect(Common::SeekableReadStream *stream) {
	int16 left = stream->readUint32LE();
	int16 top = stream->readUint32LE();
	int16 right = stream->readUint32LE();
	int16 bottom = stream->readUint32LE();
	if (left > right || top > bottom)
		return Common::Rect{{left, top}, 0, 0};
	else
		return Common::Rect{{left, top}, {right, bottom}};
}

Gui::Control *Gui::Dialog::findControlById(int id) {
	auto it = Common::find_if(
		_controls.begin(), _controls.end(),
		[id](Control &c) { return c._id == id; });
	if (it != _controls.end())
		return it;
	return nullptr;
}

Gui::Dialog *Gui::Dialog::findDialogById(int id) {
	auto it = Common::find_if(
		_dialogs.begin(), _dialogs.end(),
		[id](const Dialog &d) {
			return d._id == id;
		});
	if (it != _dialogs.end())
		return it;
	return nullptr;
}

Gui::Gui(const Common::String &filename, const Common::String &palette)
	: _palette{palette}, _filename{filename} {
	Common::ScopedPtr<Common::SeekableReadStream> stream;
	stream.reset(SearchMan.createReadStreamForMember(Common::Path{filename}));
	_rootDialog = readDialog(stream.get(), true);
	auto *meters = _rootDialog.findDialogById(IDCTL_METERS);
	auto *health = meters->findControlById(IDCTL_METERS_HEALTH);
	_fullHealthBarWidth = health->_rect.width();
	auto *exp = meters->findControlById(IDCTL_METERS_EXP);
	_fullExpBarWidth = exp->_rect.width();
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
	drawDialog(_rootDialog._rect.origin(), _rootDialog);
}

void Gui::update() {
	auto *connor = g_engine->connor();
	auto *palette = g_engine->graphicsManager().getPalette(_palette);
	const auto &reference = g_engine->reference();
	auto *empty = g_engine->graphicsManager().loadBitmap(reference._guiTagsById[IDBMP_1EMPTY], palette);

	auto *invBar = _rootDialog.findDialogById(IDCTL_INVBART);
	invBar->findControlById(IDCTL_INVBAR_GOLDTEXT)->_label = Common::String::format("%d", connor->inventoryCount(reference.itemType("INVITEM_SilverCoins")));
	invBar->findControlById(IDCTL_INVBAR_GOLD)->_gfxBitmap = g_engine->graphicsManager().loadBitmap(reference._guiTagsById[IDBMP_2GOLD3], palette);

	auto &invBarT = invBar->_dialogs[0];
	auto inventoryIt = _orderedInventory.begin();
	for (int id : {IDCTL_INVBAR_QUEST1, IDCTL_INVBAR_QUEST2, IDCTL_INVBAR_QUEST3, IDCTL_INVBAR_QUEST4, IDCTL_INVBAR_QUEST5, IDCTL_INVBAR_QUEST6, IDCTL_INVBAR_QUEST7}) {
		auto *ctl = invBarT.findControlById(id);
		if (inventoryIt == _orderedInventory.end()) {
			ctl->_gfxBitmap = empty;
		} else {
			ctl->_gfxBitmap = (*inventoryIt)->_guiBitmap;
			inventoryIt++;
		}
	}

	auto *invBarB = _rootDialog.findDialogById(IDCTL_HEALTHBAR);
	auto healingItem = [=](int bitmapControl, int textControl, const ItemType *item) {
		auto count = connor->inventoryCount(item);
		if (count > 0) {
			invBarB->findControlById(bitmapControl)->_gfxBitmap = item->_guiBitmap;
			invBarB->findControlById(textControl)->_label = Common::String::format("%d", count);
		} else {
			invBarB->findControlById(bitmapControl)->_gfxBitmap = empty;
			invBarB->findControlById(textControl)->_label = "";
		}
	};

	healingItem(IDCTL_HEALTHBAR_1, IDCTL_HEALTHBAR_1TEXT, reference.itemType("INVITEM_Mushroom"));
	healingItem(IDCTL_HEALTHBAR_2, IDCTL_HEALTHBAR_2TEXT, reference.itemType("INVITEM_Crystal"));
	healingItem(IDCTL_HEALTHBAR_3, IDCTL_HEALTHBAR_3TEXT, reference.itemType("INVITEM_SacredWater"));
	healingItem(IDCTL_HEALTHBAR_4, IDCTL_HEALTHBAR_4TEXT, reference.itemType("INVITEM_ElixerOfLife"));
	healingItem(IDCTL_HEALTHBAR_5, IDCTL_HEALTHBAR_5TEXT, reference.itemType("INVITEM_Invulnerable"));
	healingItem(IDCTL_HEALTHBAR_6, IDCTL_HEALTHBAR_6TEXT, reference.itemType("INVITEM_Strength"));
	healingItem(IDCTL_HEALTHBAR_7, IDCTL_HEALTHBAR_7TEXT, reference.itemType("INVITEM_Clarity"));
	healingItem(IDCTL_HEALTHBAR_8, IDCTL_HEALTHBAR_8TEXT, reference.itemType("INVITEM_Invisible"));

	auto *meters = _rootDialog.findDialogById(IDCTL_METERS);
	auto *level = meters->findControlById(IDCTL_METERS_LEVEL);
	level->_label = Common::String::format("%d", connor->level());
	auto *health = meters->findControlById(IDCTL_METERS_HEALTH);
	health->_rect.setWidth(_fullHealthBarWidth * connor->health() / connor->maxHealth());
	auto *exp = meters->findControlById(IDCTL_METERS_EXP);
	exp->_rect.setWidth(_fullExpBarWidth * connor->experienceAsFractionOfLevel());
}

void Gui::notifyAddToConnorInventory(const ItemType *itemType, uint16 quantity, uint16 newQuantity) {
	auto it = Common::find(_orderedInventory.begin(), _orderedInventory.end(), itemType);
	if (itemType->_category == "QuestItem" && _orderedInventory.end() == it) {
		_orderedInventory.push_back(itemType);
	}
	update();
}

void Gui::notifyRemoveFromConnorInventory(const ItemType *itemType, uint16 quantity, uint16 newQuantity) {
	if (newQuantity == 0) {
		auto it = Common::find(_orderedInventory.begin(), _orderedInventory.end(), itemType);
		if (it != _orderedInventory.end()) {
			_orderedInventory.erase(it);
		}
	}
	update();
}

void Gui::drawDialog(Common::Point offset, const Dialog &dialog) {
	if (dialog._id == 70 || dialog._id == 770)
		return;
	if (dialog._tag == ControlType::kBitmapDialog) {
		g_engine->graphicsManager().drawBitmap(dialog._gfxBitmap, dialog._rect);
	}
	auto translated = [&offset](Common::Rect r) -> Common::Rect {
		r.translate(offset.x, offset.y);
		return r;
	};

	for (const auto &control : dialog._controls) {
		if (!control._bitmap.empty()) {
			g_engine->graphicsManager().drawBitmap(control._gfxBitmap, translated(control._rect));
		}
		if (!control._font.empty() && !control._label.empty()) {
			g_engine->graphicsManager().drawText(control._gfxFont, control._label, translated(control._rect).origin());
		}
	}

	for (auto &d : dialog._dialogs) {
		drawDialog(offset + d._rect.origin(), d);
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
		dialog._bitmap = g_engine->reference()._guiTagsById.getValOrDefault(stream->readSint32LE());
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

	const auto &guiTags = g_engine->reference()._guiTagsById;
	switch (tag) {
	default:
		warning("unknown tag %x, assigning text", tag); // fallthrough
	case MKTAG('U', 'I', 'T', 'X'):
		item._tag = ControlType::kText;
		item._font = guiTags.getValOrDefault(stream->readSint32LE());
		/* auto justification = */ stream->readUint32LE();
		item._label = guiTags.getValOrDefault(stream->readUint32LE());
		break;
	case MKTAG('U', 'I', 'B', 'M'):
		item._tag = ControlType::kBitmap;
		item._bitmap = guiTags.getValOrDefault(stream->readSint32LE());
		/* auto modifier = */ stream->readUint32LE();
		/* auto attribute = */ stream->readUint32LE();
		/* auto reserved = */ stream->readUint32LE();
		break;
	case MKTAG('C', 'C', 'B', 'B'):
		item._tag = ControlType::kButton;
		item._font = guiTags.getValOrDefault(stream->readSint32LE());
		/* auto justification = */ stream->readUint32LE();
		item._label = guiTags.getValOrDefault(stream->readUint32LE());
		/* auto bitmapArrayTag = */ stream->readSint32LE();
		/* auto modifier = */ stream->readUint32LE();
		/* auto attribute = */ stream->readUint32LE();
		/* auto reserved = */ stream->readUint32LE();
		break;
	}

	return item;
}

} // namespace Kq8