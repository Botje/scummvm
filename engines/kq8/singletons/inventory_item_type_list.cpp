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

#include "kq8/singletons/inventory_item_type_list.h"

#include "kq8/kq8.h"
#include "kq8/script_tokenizer.h"

namespace Kq8 {

using namespace INIHelpers;

template<>
inline ItemType INIHelpers::extractFromValue<ItemType>(const Common::String &line) {
	ScriptTokenizer t{line};
	ItemType ret;

	/* classType = */ t.nextToken();
	ret._idName = t.nextToken();
	ret._id = t.nextUint16();
	ret._health = t.nextUint16();
	ret._sDam = t.nextUint16();
	ret._lDam = t.nextUint16();
	ret._prot = t.nextUint16();
	ret._exp = t.nextUint16();
	ret._comb = t.nextUint16();
	ret._category = t.nextToken();
	ret._guiBitmapName = t.nextToken();
	uint8 n = t.nextUint8();
	uint8 v = t.nextUint8();
	uint8 c = t.nextUint8();
	uint8 s = t.nextUint8();
	ret._message = MKTAG(n, v, c, s);
	ret._mode = t.nextToken();
	ret._duration = t.nextInt16();
	ret._disp = t.nextUint16();
	ret._landSound = t.nextToken();
	ret._pickupSound = t.nextToken();
	ret._useSound = t.nextToken();
	ret._shapeFile = t.nextToken();
	ret._cursor = t.nextToken();
	ret._radius = t.nextUint16();
	ret._translucency = t.nextUint16();
	ret._extraData = t.nextToken();

	return ret;
}
Object *Singleton::loadInventoryItemTypeList(const KQFile &ini) {
	auto items = getArray<ItemType>(ini.getSections().front(), "numItems", "item");
	auto &itemTypes = g_engine->reference()._itemTypes;
	for (const auto &item : items) {
		itemTypes.setVal(item._idName, item);
	}

	return nullptr;
}

} // namespace Kq8