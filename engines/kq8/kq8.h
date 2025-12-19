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

#ifndef KQ8_H
#define KQ8_H

#include "common/error.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "engines/engine.h"

#include "kq8/detection.h"
#include "kq8/gfx_base.h"
#include "kq8/gfx_opengls.h"
#include "kq8/graphics_manager.h"
#include "kq8/main_screen.h"
#include "kq8/objects/object_factory.h"
#include "kq8/script.h"
#include "kq8/world.h"

namespace Kq8 {

struct Kq8GameDescription;

class Kq8Engine : public Engine {
public:
	enum class GameMode {
		MainScreen,
		Game,
	};

private:
	GameMode _gameMode;
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Script::Environment _environment;
	Common::HashMap<int, Common::String> _guiTags;
	Common::ScopedPtr<MainScreen> _mainScreen;
	ObjectFactory _objectFactory;
	GraphicsManager _graphicsManager;
	GfxBase *_gfx;
	Common::ScopedPtr<World> _world;

	void loadGuiTags();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Kq8Engine(OSystem *syst, const ADGameDescription *gameDesc);
	~Kq8Engine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	}

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	Common::String getGuiTag(uint32 value);
	GraphicsManager &graphicsManager() { return _graphicsManager; }
	GfxBase &gfx() { return *_gfx; }
	World *world() { return _world.get(); }
	ObjectFactory &objectFactory() { return _objectFactory; }
	const Common::String &getVariable(const Common::String &variable) { return _environment.getValOrDefault(variable); }

	void runScript(const Common::String &file, const Script::Args &args);
	void setWorld(const Common::String &world, const Common::String &parent = "");
};

extern Kq8Engine *g_engine;
#define SHOULD_QUIT ::Kq8::g_engine->shouldQuit()

} // End of namespace Kq8

#endif // KQ8_H
