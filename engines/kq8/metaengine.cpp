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

#include "common/translation.h"

#include "kq8/metaengine.h"
#include "kq8/detection.h"
#include "kq8/kq8.h"

namespace Kq8 {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Kq8

const char *Kq8MetaEngine::getName() const {
	return "kq8";
}

const ADExtraGuiOptionsMap *Kq8MetaEngine::getAdvancedExtraGuiOptions() const {
	return Kq8::optionsList;
}

Common::Error Kq8MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Kq8::Kq8Engine(syst, desc);
	return Common::kNoError;
}

bool Kq8MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(KQ8)
REGISTER_PLUGIN_DYNAMIC(KQ8, PLUGIN_TYPE_ENGINE, Kq8MetaEngine);
#else
REGISTER_PLUGIN_STATIC(KQ8, PLUGIN_TYPE_ENGINE, Kq8MetaEngine);
#endif
