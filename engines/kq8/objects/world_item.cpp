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
 
 #include "kq8/objects/world_item.h"
 
 #include "kq8/kq8.h"
 #include "kq8/objects/connor.h"
 
 namespace Kq8 {
 
 WorldItem::WorldItem(const Common::String &itemTypeName, uint16 quantity) : Object{g_engine->generateName(itemTypeName)}, _quantity{quantity} {
 
	_itemType = &g_engine->reference()._itemTypes[Common::String::format("INVITEM_%s", itemTypeName.c_str())];
	_itemType = g_engine->reference().itemType(itemTypeName);
 	_shape = g_engine->graphicsManager().loadshape(_itemType->_shapeFile);
 }
 
 void WorldItem::sendEvent(const Common::String &eventType, const Script::Args &args) {
 	Object::sendEvent(eventType, args);
 	if (eventType == "ConnorAction") {
 		g_engine->world()->deleteLater(this);
 		g_engine->world()->connor()->addToInventory(_itemType, _quantity);
 	}
 }
 
 } // namespace Kq8
