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

#include "chest.h"

#include "kq8/kq8.h"

namespace Kq8 {

using namespace INIHelpers;

Object *Chest::factory(const KQFile &f) {
	return new Chest(f);
}

Chest::Chest(const KQFile &f) : AnimObject{f} {
	const auto &section = f.getSections().front();

	auto invItemTypeName = get<Common::String>(section, "invItemTypeName");
	auto itemType = g_engine->reference().itemType(invItemTypeName);
	auto invQuantity = get<uint16>(section, "invQuantity");
	auto invLoc = get(section, "invLocX", "invLocY", "invLocZ");
	auto invRot = get(section, "invDirX", "invDirY", "invDirZ");

	_emptyMsg = get<uint8>(section, "EmptyMsg");
	_emptyNoun = get<uint8>(section, "EmptyNoun");
	_emptyVerb = get<uint8>(section, "EmptyVerb");
	_emptyCase = get<uint8>(section, "EmptyCase");
	_emptySeq = get<uint8>(section, "EmptySeq");
	_emptyEnd = get<uint8>(section, "EmptyEnd");

	_item.reset(new WorldItem{itemType, invQuantity});
	_item->moveTo(invLoc);
	_item->setRotation(invRot);
}

void Chest::sendEvent(const Common::String &eventType, const Script::Args &args) {
	if (_item) {
		g_engine->world()->addObject(_item.release());
	} else {
		auto *connor = g_engine->connor();
		connor->speak(1000, _emptyNoun, _emptyVerb, _emptyCase, _emptySeq, _emptyEnd);
	}
}

} // namespace Kq8