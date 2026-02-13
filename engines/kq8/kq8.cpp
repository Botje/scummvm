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

#include "audio/decoders/sol.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"

#include "kq8/console.h"
#include "kq8/detection.h"
#include "kq8/gfx_base.h"
#include "kq8/gfx_opengls.h"
#include "kq8/gui.h"
#include "kq8/kq8.h"
#include "kq8/objects/camera.h"
#include "kq8/script.h"

namespace Kq8 {

Kq8Engine *g_engine;

Kq8Engine::Kq8Engine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _gameMode(GameMode::Game), _gameDescription(gameDesc), _randomSource("Kq8") {
	g_engine = this;

	_environment.setVal("KQGame::BitDepth", "8");
	_environment.setVal("KQGame::Language", "english");
	_environment.setVal("KQGame::runOptimal", "yes");

	auto root = ConfMan.getPath("path");
	auto game = root.join("game");
	SearchMan.addDirectory("game_8gui", game.join("8gui"), 10000);
	SearchMan.addDirectory("game_common", game.join("common"), 10000);
	SearchMan.addDirectory("game_english", game.join("english"), 10000);
	SearchMan.addDirectory("game_kq", game.join("kq"), 10000);
	SearchMan.addDirectory("game_resource", game.join("resource"), 10000);
	SearchMan.addDirectory("game_sound", game.join("sound"), 10000);

	SearchMan.addDirectory("game_patch", game.join("patch"), 9000);

	DebugMan.enableDebugChannel(kDebugSpeech);
	DebugMan.enableAllDebugChannels();
}

Kq8Engine::~Kq8Engine() {
}

uint32 Kq8Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String Kq8Engine::getGameId() const {
	return _gameDescription->gameId;
}

ItemType *Kq8Engine::Reference::itemType(const Common::String &t) {
	auto prefixed = t.hasPrefix("INVITEM_") ? t : Common::String::format("INVITEM_%s", t.c_str());
	if (!_itemTypes.contains(prefixed)) {
		error("Attempt to look up invalid item type %s", prefixed.c_str());
	}
	return &_itemTypes[t];
}

// This file starts with [TAG] end ends with [END].
// The lines we care about are of the form
//    00000725 IDSTR_CANCEL '#35 Cancel'
void Kq8Engine::loadGuiTags() {
	auto stream = SearchMan.createReadStreamForMember("kqGuiTag.TTAG");
	if (!stream) {
		error("Could not load 'kqGuiTag.TTAG' file");
	}

	/* auto tag = */ stream->readLine();
	while (true) {
		auto line = stream->readLine();
		if (stream->eos() || line == "[END]") {
			break;
		}

		auto firstSpace = line.find(' ');
		auto firstQuote = line.find('\'');
		auto lastQuote = line.rfind('\'');
		if (firstSpace == Common::String::npos || firstQuote == Common::String::npos || lastQuote == Common::String::npos || firstQuote == lastQuote) {
			warning("Could not parse line %s", line.c_str());
		} else {
			auto key = line.substr(0, firstSpace).asUint64();
			auto value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
			_guiTags[key] = value;
		}
	}
}

void Kq8Engine::handleKey(Common::KeyCode keycode, bool isDown) {
#define CASE(keycode, flag)     \
	case keycode:               \
		if (isDown) {           \
			_inputs |= flag;    \
		} else {                \
			_inputs &= ~(flag); \
		}                       \
		break;

	switch (keycode) {
		CASE(Common::KEYCODE_RIGHT, Input::kRight);
		CASE(Common::KEYCODE_LEFT, Input::kLeft);
		CASE(Common::KEYCODE_UP, Input::kForward);
		CASE(Common::KEYCODE_DOWN, Input::kBackward);
	default:
		break;
	}
#undef CASE
#define UNDO_OPPOSITE(mask)           \
	if ((mask) == (_inputs & (mask))) \
		_inputs &= ~(mask);
	UNDO_OPPOSITE(Input::kRight | Input::kLeft);
	UNDO_OPPOSITE(Input::kForward | Input::kBackward);
#undef UNDO_OPPOSITE
}

Common::String Kq8Engine::getGuiTag(uint32 value) {
	return _guiTags.getValOrDefault(value);
}

void Kq8Engine::drawMouseCursor() {
	_gfx->drawBitmap(_mouseBitmap, Common::Rect::center(_mousePos.x, _mousePos.y, _mouseBitmap->surface()->w, _mouseBitmap->surface()->h));
}
Common::Error Kq8Engine::run() {
	initGraphics3d(640, 480);
	_gfx = new GfxOpenGLS();

	// Set the engine's debugger console
	setDebugger(new Console());

	loadGuiTags();

	auto menusPalette = graphicsManager().getPalette("Menus.ppl");
	_consoleFont = graphicsManager().loadFont("console1.pft", menusPalette);

	runScript("Mask.cs", Script::Args{"_", "Init"});
	setWorld("daventry");

	queueScript("World.cs", Script::Args{"_", "Begin"});
	// runScript("worldVar.cs", Script::Args{"_"});

	_gui.reset(new Gui("gplayscr.gui", "menus.ppl"));
	_gui->prepare();

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	_mouseBitmap = graphicsManager().loadBitmap("curs04.pba", menusPalette);
	const Object *pointingAt = nullptr;

	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_MOUSEMOVE: {
				_mousePos = e.mouse;
				if (_inputs & Input::kPanCamera) {
					world()->camera()->pan(Math::Vector2d(e.relMouse.x, e.relMouse.y) / Math::Vector2d(_system->getWidth(), _system->getHeight()));
				} else {
					pointingAt = gfx().mousePick(Common::Point{_mousePos.x, static_cast<short>(g_system->getHeight() - _mousePos.y)});
				}
				break;
			}

			case Common::EVENT_LBUTTONUP: {
				if (pointingAt) {
					const_cast<Object *>(pointingAt)->sendEvent("ConnorAction", {"do", "do"});
				}
				break;
			}

			case Common::EVENT_WHEELDOWN: {
				world()->camera()->zoomIn();
				break;
			}
			case Common::EVENT_WHEELUP: {
				world()->camera()->zoomOut();
				break;
			}

			case Common::EVENT_RBUTTONDOWN: {
				_inputs |= Input::kPanCamera;
				pointingAt = nullptr;
				break;
			}
			case Common::EVENT_RBUTTONUP: {
				_inputs = _inputs & ~Input::kPanCamera;
				break;
			}

			case Common::EVENT_KEYDOWN:
			case Common::EVENT_KEYUP: {
				handleKey(e.kbd.keycode, e.type == Common::EVENT_KEYDOWN);
				break;
			}

			default:
				break;
			}
		}

		decltype(_queuedScripts) localQueuedScripts;
		_queuedScripts.swap(localQueuedScripts);

		auto it = _timedEvents.begin();
		while (it != _timedEvents.end() && it->first <= _system->getMillis()) {
			auto &timedEvent = it->second;

			Script::Args args{timedEvent._obj->name(), timedEvent._eventType};
			args.insert_at(args.size(), timedEvent._eventArgs);
			const auto &script = timedEvent._obj->script();
			localQueuedScripts.push_back({script, args});

			it = _timedEvents.erase(it);
		}
		for (const auto &p : localQueuedScripts) {
			runScript(p.first, p.second);
		}

		_gfx->clearScreen();
		switch (_gameMode) {
		case GameMode::MainScreen:
			break;
		case GameMode::Game: {
			gfx().setupCamera();
			if (_world) {
				_world->update(1.f / 60.f);
				_world->draw();
			}
			gfx().setupOverlay();
			_gui->draw();
			drawMouseCursor();
			if (pointingAt) {
				graphicsManager().drawText(_consoleFont, pointingAt->name(), _mousePos);
			}
			break;
		}
		}

		_gfx->flipBuffer();

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

void Kq8Engine::queueScript(const Common::String &file, const Script::Args &args) {
	_queuedScripts.emplace_back(file, args);
}

void Kq8Engine::queueEvent(Object *obj, const Common::String &eventType, const Script::Args &eventArgs, uint32 delay) {
	uint32 target = _system->getMillis() + delay;
	_timedEvents.insert({target, TimedEvent{obj, eventType, eventArgs}});
}

void Kq8Engine::runScript(const Common::String &file, const Script::Args &args) {
	if (file == "<inline>") {
		Script{Script::InlineMarker{}, args}.evaluate(_environment, {});
	} else {
		Script{file}.evaluate(_environment, args);
	}
}

void Kq8Engine::setWorld(const Common::String &world, const Common::String &parent) {
	_world.reset(new World(world));
	_environment.setVal("KQWorld::Parent", parent); // TODO: also load parent resources?
	_environment.setVal("KQWorld::Name", world);

	auto root = ConfMan.getPath("path");
	auto game = root.join(world);
	SearchMan.remove("world_8gui");
	SearchMan.remove("world_8bit");
	SearchMan.remove("world_english");
	SearchMan.remove("world_kq");
	SearchMan.remove("world_kq_light");
	SearchMan.remove("world_kq_terrain");
	SearchMan.remove("world_resource");
	SearchMan.remove("world_sound");
	SearchMan.remove("world_patch");

	SearchMan.addDirectory("world_8gui", game.join("8gui"), 6000);
	SearchMan.addDirectory("world_8bit", game.join("8bit"), 6000);
	SearchMan.addDirectory("world_english", game.join("english"), 6000);
	SearchMan.addDirectory("world_kq", game.join("kq"), 6000);
	SearchMan.addDirectory("world_kq_light", game.join("kq").join("light"), 6000);
	SearchMan.addDirectory("world_kq_terrain", game.join("kq").join("terrain"), 6000);
	SearchMan.addDirectory("world_resource", game.join("resource"), 6000);
	SearchMan.addDirectory("world_sound", game.join("sound"), 6000);

	SearchMan.addDirectory("world_patch", game.join("patch"), 5000);

	runScript("mask.cs", Script::Args{"_", "NewWorld"});
}

void Kq8Engine::notifyAnimationEnded(Object *obj, const Common::String &animation) {
	const auto start = _animationEndSubscriptions.lower_bound(obj->name());
	const auto end = _animationEndSubscriptions.upper_bound(obj->name());
	for (auto it = start; it != end; ++it) {
		const auto &receiver = it->second;
		auto *receiverObject = world()->findObject(receiver);
		if (!receiverObject)
			continue;

		const Script::Args args{
			receiver,
			"Cue",
			"300",
			"_",
			animation};
		queueScript(receiverObject->script(), args);
	}
}
void Kq8Engine::subscribeAnimationEnd(const Common::String &origin, const Common::String &receiver) {
	_animationEndSubscriptions.insert({origin, receiver});
}
void Kq8Engine::unsubscribeAnimationEnd(const Common::String &origin, const Common::String &receiver) {
	const auto start = _animationEndSubscriptions.lower_bound(origin);
	const auto end = _animationEndSubscriptions.upper_bound(origin);
	for (auto it = start; it != end; ++it) {
		if (it->second == receiver) {
			_animationEndSubscriptions.erase(it);
			return;
		}
	}
}

Audio::SoundHandle Kq8Engine::playSound(const Common::String &file, Audio::Mixer::SoundType soundType) {
	auto *stream = SearchMan.createReadStreamForMember(Common::Path{file});
	if (!stream) {
		warning("Could not find sound file %s", file.c_str());
		return {};
	}

	auto *audioStream = Audio::makeSOLStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		delete stream;
		warning("Could not process sound file %s", file.c_str());
		return {};
	}

	_system->getMixer()->playStream(soundType, &_soundHandles[file], audioStream);
	return _soundHandles[file];
}
Common::String Kq8Engine::generateName(const Common::String &prefix) {
	return Common::String::format("%s[%08x]", prefix.c_str(), getRandomNumber(UINT_MAX));
}

} // End of namespace Kq8
