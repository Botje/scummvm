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

#include "audio/mixer.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/multimap.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "engines/engine.h"

#include "kq8/detection.h"
#include "kq8/gfx_base.h"
#include "kq8/gfx_opengls.h"
#include "kq8/graphics_manager.h"
#include "kq8/gui.h"
#include "kq8/input.h"
#include "kq8/objects/object_factory.h"
#include "kq8/script.h"
#include "kq8/singletons/gui_tags.h"
#include "kq8/singletons/inventory_item_type_list.h"
#include "kq8/singletons/monster_type_list.h"
#include "kq8/world.h"

namespace Kq8 {

class Camera;
class Connor;
struct Kq8GameDescription;

class Kq8Engine : public Engine {
public:
	enum class GameMode {
		MainScreen,
		Game,
	};

	enum class CursorMode : uint8 {
		MeleeAttack = 0,
		RangedAttack = 1,
		HandsOff = 2,
		Unknown = 3,
		Do = 4,
	};

	struct Reference {
		Common::HashMap<Common::String, ItemType, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _itemTypes;
		Common::HashMap<Common::String, Object::BoundingBox, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _boundingBoxes;
		const ItemType *itemType(const Common::String &t) const;
		Common::HashMap<Common::String, MonsterType> _monsterTypes;
		Common::HashMap<Common::String, Script::OpcodeFn> _opcodes;
		GuiTagsById _guiTagsById;
		GuiTagsByName _guiTagsByName;
	};

private:
	GameMode _gameMode;
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Script::Environment _environment;
	Common::ScopedPtr<Gui> _gui;
	ObjectFactory _objectFactory;
	GraphicsManager _graphicsManager;
	GfxBase *_gfx;
	Common::ScopedPtr<World> _world;
	Common::Array<Common::Pair<Common::String, Script::Args>> _queuedScripts;
	Common::MultiMap<Common::String, Common::String> _animationEndSubscriptions;

	CursorMode _cursorMode = CursorMode::Do;
	Common::Point _mousePos;
	const Bitmap *_mouseBitmaps[5];

	Font *_consoleFont;
	Reference _reference;
	Common::HashMap<Common::String, Common::Array<byte>> _worldStates;

	struct TimedEvent {
		Object *_obj;
		const Common::String _eventType;
		const Script::Args _eventArgs;
	};
	Common::MultiMap<uint32, TimedEvent> _timedEvents;
	Common::HashMap<Common::String, Audio::SoundHandle> _soundHandles;
	uint32 _inputs = Input::kNone;
	void handleKey(Common::KeyCode keycode, bool isDown);

	Common::ScopedPtr<Camera> _camera;
	Common::ScopedPtr<Connor> _connor;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	static void drawDebugConsole();
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

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	void drawMouseCursor();
	void debugDraw();
	GraphicsManager &graphicsManager() { return _graphicsManager; }
	GfxBase &gfx() { return *_gfx; }
	World *world() { return _world.get(); }
	ObjectFactory &objectFactory() { return _objectFactory; }
	const Common::String &getVariable(const Common::String &variable) { return _environment.getValOrDefault(variable); }
	uint32 inputs() const { return _inputs; }
	Reference &reference() { return _reference; }
	Camera *camera() { return _camera.get(); }
	Connor *connor() { return _connor.get(); }
	void setConnor(Connor *connor) { _connor.reset(connor); };
	CursorMode cursorMode() const { return _cursorMode; }
	void setCursorMode(CursorMode mode) { _cursorMode = mode; }

	void queueScript(const Common::String &file, const Script::Args &args);
	void queueEvent(Object *obj, const Common::String &string, const Script::Args &args, uint32 delay);
	void runScript(const Common::String &file, const Script::Args &args);
	void setWorld(const Common::String &world, const Common::String &parent = "");
	void notifyAnimationEnded(Object *obj, const Common::String &string);
	void notifyAddToConnorInventory(const ItemType *itemType, uint16 quantity, uint16 newQuantity);
	void notifyRemoveFromConnorInventory(const ItemType *itemType, uint16 quantity, uint16 newQuantity);
	void notifyConnorStatsChange();
	void subscribeAnimationEnd(const Common::String &origin, const Common::String &receiver);
	void unsubscribeAnimationEnd(const Common::String &origin, const Common::String &receiver);

	Audio::SoundHandle playSound(const Common::String &string, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	Common::String generateName(const Common::String &prefix);
};

extern Kq8Engine *g_engine;
#define SHOULD_QUIT ::Kq8::g_engine->shouldQuit()

} // End of namespace Kq8

#endif // KQ8_H
