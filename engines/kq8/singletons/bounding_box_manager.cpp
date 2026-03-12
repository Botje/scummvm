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

#include "kq8/singletons/bounding_box_manager.h"

#include "kq8/kq8.h"
#include "kq8/script_tokenizer.h"

namespace Kq8 {

using namespace INIHelpers;

namespace Singleton {

static Math::Vector3d vec3FromLine(ScriptTokenizer &t) {
	Math::Vector3d ret;
	ret.x() = t.nextFloat();
	ret.y() = t.nextFloat();
	ret.z() = t.nextFloat();
	return ret;
}

Object *loadBoundingBoxManager(const KQFile &ini) {
	const auto &section = ini.getSections().front();
	Common::HashMap<Common::String, Object::BoundingBox, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> boundingBoxes;
	for (const auto &line : getArray<Common::String>(section, "nItems", "item")) {
		ScriptTokenizer t{line};
		auto key = t.nextToken();
		auto min = vec3FromLine(t);
		auto max = vec3FromLine(t);
		boundingBoxes[key] = Object::BoundingBox{min, max};
	}
	g_engine->reference()._boundingBoxes = Common::move(boundingBoxes);
	return nullptr;
}

} // namespace Singleton
} // namespace Kq8
