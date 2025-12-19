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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"

#include "kq8/kq8.h"
#include "kq8/script.h"
#include "kq8/detection.h"
#include "kq8/console.h"

namespace Kq8 {

Kq8Engine *g_engine;

Kq8Engine::Kq8Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Kq8") {
	g_engine = this;

	_environment.setVal("KQGame::BitDepth", "8");
	_environment.setVal("KQGame::Language", "english");
	_environment.setVal("KQGame::runOptimal", "yes");

	auto root = ConfMan.getPath("path");
	auto game = root.join("game");
	SearchMan.addDirectory(game.join("8gui"), 10000);
	SearchMan.addDirectory(game.join("common"), 10000);
	SearchMan.addDirectory(game.join("english"), 10000);
	SearchMan.addDirectory(game.join("kq"), 10000);
	SearchMan.addDirectory(game.join("resource"), 10000);
	SearchMan.addDirectory(game.join("sound"), 10000);

	SearchMan.addDirectory(game.join("patch"), 9000);
}

Kq8Engine::~Kq8Engine() {
}

uint32 Kq8Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String Kq8Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error Kq8Engine::run() {
	initGraphics3d(640, 480);

	// Set the engine's debugger console
	setDebugger(new Console());


	Script{Common::String{"Mask.cs"}}.evaluate(_environment, Script::Args{"_", "Init"});

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error Kq8Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Kq8
