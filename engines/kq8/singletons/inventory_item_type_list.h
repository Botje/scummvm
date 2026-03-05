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

#ifndef KQ8_INVENTORY_ITEM_TYPE_LIST_H
#define KQ8_INVENTORY_ITEM_TYPE_LIST_H

#include "kq8/bitmap.h"
#include "kq8/kq_file.h"
#include "kq8/msg_file.h"
#include "kq8/objects/object.h"

namespace Kq8 {

using NVCS = MsgFile::NVCS;

struct ItemType {
	Common::String _idName;
	uint16 _id;
	uint16 _health;
	uint16 _sDam;
	uint16 _lDam;
	uint16 _prot;
	uint16 _exp;
	uint16 _comb;
	Common::String _category;
	Common::String _guiBitmapName;
	Bitmap *_guiBitmap;
	NVCS _message;
	Common::String _mode;
	int16 _duration;
	uint16 _disp;
	Common::String _landSound;
	Common::String _pickupSound;
	Common::String _useSound;
	Common::String _shapeFile;
	Common::String _cursor;
	uint16 _radius;
	uint16 _translucency;
	Common::String _extraData;
};

namespace Singleton {
Object *loadInventoryItemTypeList(const KQFile &ini);
}

} // namespace Kq8

#endif // KQ8_INVENTORY_ITEM_TYPE_LIST_H
