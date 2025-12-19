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

#include "common/debug.h"
#include "common/fs.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/tokenizer.h"

#include "kq8/kq8.h"
#include "kq8/kq_file.h"
#include "kq8/objects/connor.h"
#include "kq8/script.h"

namespace Kq8 {

static const Common::String EMPTY_TOKEN = "<EMPTY>";
using Tokens = Common::Array<Common::String>;

#define trace_entry() debugC(kDebugScript, "%s:%d > %s %s", _name.c_str(), expr->line(), expr->tokenAt(0).c_str(), joinArgs(args).c_str())
#define trace(msg, ...) debugC(kDebugScript, "%s:%d " msg, _name.c_str(), expr->line(), __VA_ARGS__)
#define trace_(msg) debugC(kDebugScript, "%s:%d " msg, _name.c_str(), expr->line())
#define traceWarn(msg, ...) debugC(kDebugScript, "%s:%d %s: " msg, _name.c_str(), expr->line(), expr->tokenAt(0).c_str(), __VA_ARGS__)
#define traceWarn_(msg) debugC(kDebugScript, "%s:%d %s: " msg, _name.c_str(), expr->line(), expr->tokenAt(0).c_str())

static Common::String joinArgs(const Script::Args &args) {
	Common::String result;
	for (auto &a : args) {
		result += a.empty() ? "''" : a;
		if (&a != &args.back())
			result += " ";
	}
	return result;
}

struct Tokenizer {
	explicit Tokenizer(const Common::String &str) : str_(str), pos_(str.begin()) {
		reset();
	}
	const Common::String &str_;
	Common::String::const_iterator pos_;

	void advance() {
		while (pos_ != str_.end() && Common::isSpace(*pos_)) {
			pos_++;
		}
	}

	void reset() {
		pos_ = str_.begin();
		advance();
	}

	Tokens rest() {
		advance();
		Tokens ret;
		Common::String tok;
		while ((tok = nextToken()) != EMPTY_TOKEN) {
			ret.push_back(tok);
		}
		return ret;
	}

	Common::String nextToken() {
		advance();

		if (pos_ == str_.end() || *pos_ == '#') {
			pos_ = str_.end();
			return EMPTY_TOKEN;
		}

		if (*pos_ == '"' || *pos_ == '\'') {
			char startQuote = *pos_;
			pos_++;
			auto start = pos_;
			while (pos_ != str_.end() && *pos_ != startQuote) {
				pos_++;
			}

			// TODO: this ignores unbalanced quotes. TBD if this is a problem
			return Common::String(start, pos_++);
		} else {
			auto start = pos_;
			while (pos_ != str_.end() && !Common::isSpace(*pos_) && *pos_ != '#') {
				pos_++;
			}
			return Common::String(start, pos_);
		}
	}
};

struct Expr {
	virtual ~Expr() = default;
};

struct LineExpr : public Expr {
	LineExpr(uint16 lineNumber, const Tokens &line) : _lineNumber{lineNumber}, line_(line) {}
	LineExpr(uint16 lineNumber, Tokens &&line) : _lineNumber{lineNumber}, line_(line) {}
	Tokens line_;
	uint16 _lineNumber;
	const Common::String &tokenAt(uint i) const { return line_[i]; }
	uint16 line() const { return _lineNumber; }
};

float getNumber(const Common::String &s) {
	float ret;
	sscanf(s.c_str(), "%f", &ret);
	return ret;
}

bool getBoolean(const Common::String &s) {
	return s == "1";
}

struct IfExpr : public Expr {
	IfExpr(const LineExpr &condition)
		: condition_(condition), _then(new Block), _else(new Block) {}

	~IfExpr() {
		delete _then;
		delete _else;
	}
	LineExpr condition_;
	Block *_then;
	Block *_else;
};

Block::~Block() {
	for (auto e : _body) {
		delete e;
	}
}

Script::Script(const Common::String &name)
	: _name(name) {
	uint16 lineNumber = 0;
	auto stream = SearchMan.createReadStreamForMember(Common::Path{name});
	if (!stream) {
		error("Cannot load '%s'", name.c_str());
	}
	auto blocks = Common::Stack<Block *>{};
	blocks.push(&_body);
	while (!stream->eos()) {
		lineNumber++;
		auto line = stream->readLine();
		auto commentPos = line.find('#');
		if (commentPos != line.npos) {
			line = line.substr(0, commentPos);
		}

		auto tokenizer = Tokenizer{line};
		auto token = tokenizer.nextToken();
		bool specialLine = false;
		if (token == EMPTY_TOKEN) {
			continue;
		}

		if (token == "else") {
			blocks.pop();
			auto ifExpr = reinterpret_cast<IfExpr *>(blocks.top()->_body.back());
			blocks.push(ifExpr->_else);
			token = tokenizer.nextToken();
			specialLine = true;
			// fall through to the if case
		}
		if (token == "if") {
			auto condition = LineExpr{lineNumber, tokenizer.rest()};
			auto ifExpr = new IfExpr{condition};
			blocks.top()->_body.push_back(ifExpr);
			blocks.push(ifExpr->_then);
			specialLine = true;

		} else if (token == "endif") {
			blocks.pop();
			specialLine = true;
		}
		if (!specialLine) {
			tokenizer.reset();
			blocks.top()->_body.push_back(new LineExpr{lineNumber, tokenizer.rest()});
		}
	}
}
void Script::evaluate(Script::Environment &env, const Script::Args &args) {
	debug("Evaluating %s %s", _name.c_str(), joinArgs(args).c_str());
	evaluate(env, args, _body);
}
void Script::evaluate(Script::Environment &env, const Script::Args &args, const Block &block) {
	for (auto expr : block._body) {
		auto ifExpr = dynamic_cast<IfExpr *>(expr);
		if (ifExpr) {
			auto left = evaluateExpr(env, args, ifExpr->condition_.tokenAt(1));
			auto right = evaluateExpr(env, args, ifExpr->condition_.tokenAt(3));
			if (left == right) {
				evaluate(env, args, *ifExpr->_then);
			} else {
				evaluate(env, args, *ifExpr->_else);
			}

		} else {
			auto lineExpr = dynamic_cast<LineExpr *>(expr);
			auto &first = lineExpr->line_.front();

			Script::Args actual_args;
			for (auto it = lineExpr->line_.begin() + 1; it != lineExpr->line_.end(); it++) {
				auto val = evaluateExpr(env, args, *it);
				if (it->hasPrefix("$")) {
					const auto words = Common::StringTokenizer{val}.split();
					if (words.empty()) {
						actual_args.emplace_back("");
					} else {
						actual_args.insert_at(actual_args.size(), words);
					}
				} else {
					actual_args.emplace_back(val);
				}
			}

			if (first.hasSuffix(".cs")) {
				Script{first}.evaluate(env, actual_args);
			} else {
				auto opcode = getOpcodes().getValOrDefault(first);
				if (!opcode) {
					opcode = &Script::op_missing;
				}
				(this->*opcode)(env, actual_args, lineExpr);
			}
		}
	}
}
Common::String Script::evaluateExpr(Script::Environment &env, const Script::Args &args, const Common::String &string) {
	if (string.hasPrefix("$")) {
		if (Common::isDigit(string[1])) {
			auto idx = string[1] - '1';
			return idx < args.size() ? args[idx] : ""; // $1 is the first argument
		} else {
			return env.getValOrDefault(string.substr(1));
		}
	}
	return string;
}

// TODO: refactor to not use a static table
const Common::HashMap<Common::String, Script::OpcodeFn> &Script::getOpcodes() {
	static Common::HashMap<Common::String, OpcodeFn> opcodes;
	if (opcodes.empty()) {
#define OPCODE(x) opcodes.setVal(#x, &Script::op_##x)
#define OPCODE2(x, y) opcodes.setVal(#x, &Script::op_##y)
#include "kq8/opcodes.h"

#undef OPCODE
#undef OPCODE2
		opcodes.erase("missing");
	}
	return opcodes;
}

void Script::op_missing(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	traceWarn("Opcode missing: args=[%s]", joinArgs(args).c_str());
}

void Script::op_move(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	enum MoveFlag {
		kMoveAbsRotAbs = 0,
		kMoveRelRotAbs = 1,
		kMoveAbsRotRel = 2,
		kMoveRelRotRel = 3,

	};
	auto who = args[0];
	MoveFlag flag = (MoveFlag)getNumber(args[1]);
	auto x = getNumber(args[2]);
	auto y = getNumber(args[3]);
	auto z = getNumber(args[4]);

	auto *obj = g_engine->world()->findObject(who);
	if (!obj) {
		traceWarn("Could not find object %s", who.c_str());
		return;
	}

	Math::Vector3d newPos = Math::Vector3d(x, y, z);
	if (flag == kMoveRelRotAbs || flag == kMoveRelRotRel) {
		newPos += obj->pos();
	}

	obj->moveTo(newPos);

	if (args.size() == 8) {
		auto lx = getNumber(args[5]);
		auto ly = getNumber(args[6]);
		auto lz = getNumber(args[7]);
		Math::Vector3d newRot = Math::Vector3d(lx, ly, lz);
		if (flag == kMoveRelRotRel || flag == kMoveAbsRotRel) {
			newRot += obj->rot();
		}

		obj->setRotation(newRot);
	}
}

void Script::op_set(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	env.setVal(args[0], args[1]);
}

void Script::op_sendEvent(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto delay = getNumber(args[0]);
	auto who = args[1];
	auto eventType = args[2];
	eventType = eventType.substr(2, eventType.size() - strlen("KQ") - strlen("Event"));

	auto *obj = g_engine->world()->findObject(who);
	if (!obj) {
		traceWarn("Could not find object %s", who.c_str());
		return;
	}

	auto eventParameters = Script::Args(args.begin() + 3, args.size() - 3);
	if (delay > 0) {
		g_engine->queueEvent(obj, eventType, eventParameters, 1000 * delay);
	} else {
		obj->sendEvent(eventType, eventParameters);
	}
}

void Script::op_loadKQ(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	KQFile kqFile;
	auto file = args[0];
	auto pos = file.find('/');
	if (pos != Common::String::npos) {
		file = file.substr(pos + 1);
	}
	auto stream = Common::ScopedPtr<Common::SeekableReadStream>{SearchMan.createReadStreamForMember(Common::Path{file})};

	if (!stream) {
		traceWarn("Could not loadKQ '%s'", file.c_str());
		return;
	}
	kqFile.loadFromStream(*stream);
	auto &section = kqFile.getSections().front();
	auto klass = section.getKey("classType")->value;

	auto *obj = g_engine->objectFactory().load(klass, kqFile);
	if (!obj) {
		traceWarn("Unknown object type %s", klass.c_str());
	} else {
		if (args.size() >= 2) {
			auto name = args[1];
			if (name != "same") {
				obj->setName(name);
			}
		}
		if (args.size() >= 5) {
			auto x = getNumber(args[2]);
			auto y = getNumber(args[3]);
			auto z = getNumber(args[4]);
			obj->moveTo(Math::Vector3d{x, y, z});
		}
		if (args.size() >= 8) {
			auto rx = getNumber(args[5]);
			auto ry = getNumber(args[6]);
			auto rz = getNumber(args[7]);
			obj->setRotation(Math::Vector3d{rx, ry, rz});
		}
	}
}

void Script::op_lockResource(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	// Do nothing
}
void Script::op_purgeResource(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	// Do nothing
}

void Script::op_setcat(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto variable = args[0];
	Common::String value;
	for (auto it = args.begin() + 1; it != args.end(); ++it) {
		value += *it;
	}

	env.setVal(variable, value);
}

void Script::op_setLoadProgress(Script::Environment &, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	// Do nothing
}

void Script::op_echo(Script::Environment &, const Script::Args &args, LineExpr *expr) {
	trace("%s", joinArgs(args).c_str());
}

void Script::op_getEndLoop(Script::Environment &, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto origin = args[0];
	auto receiver = args[1];
	bool enable = getBoolean(args[2]);

	auto *originObject = g_engine->world()->findObject(origin);
	auto *receiverObject = g_engine->world()->findObject(receiver);

	if (!originObject) {
		traceWarn("Could not find origin object %s", origin.c_str());
		return;
	}
	if (!receiverObject) {
		traceWarn("Could not find receiver object %s", receiver.c_str());
		return;
	}

	if (enable) {
		g_engine->subscribeAnimationEnd(origin, receiver);
	} else {
		g_engine->unsubscribeAnimationEnd(origin, receiver);
	}
}

void Script::op_alias(Script::Environment &, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	// Do nothing
}

void Script::op_KQObject__setScript(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto who = args[0];
	auto script = args[1];

	auto *obj = g_engine->world()->findObject(who);
	if (!obj) {
		traceWarn("Could not find object %s", who.c_str());
		return;
	}
	obj->setScript(script != "none" ? script : "");
}

void Script::op_KQMonster__setState(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	// Could be one of
	// - Wait
	// - Move
	// - Unborn
	// - SwapWeapons
	// - ThrowRock
	// - Jump
	// - Push
	// - Die
	// - Tile
	// - Tile2
	// - Tile3
	// - Scales
	// - Choice
	// - Portal
	// - Special
	auto who = args[0];
	auto state = args[1];
	if (who != "Connor" || state != "special") {
		traceWarn_("who != Connor or state != special");
		return;
	}

	auto *obj = g_engine->world()->findObject(who);
	if (!obj) {
		traceWarn("Could not find object %s", who.c_str());
		return;
	}

	auto extra = args[2];
	Common::StringTokenizer tok{extra, "=,"};
	/* animList= */ tok.nextToken();
	auto animListName = tok.nextToken();
	/* AnimListOn */ tok.nextToken();
	auto states = tok.split();

	auto *connor = dynamic_cast<Connor *>(obj);
	connor->startSpecialAnimation(animListName, states);
}

void Script::op_KQMonster__speak(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto talker = args[0];
	auto listener = args[1];
	int catalog = getNumber(args[2]);
	uint8 noun = getNumber(args[3]);
	uint8 verb = getNumber(args[4]);
	uint8 kase = getNumber(args[5]);
	uint8 startSeq = getNumber(args[6]);
	uint8 endSeq = getNumber(args[7]);

	// TODO: look up talker id
	auto str = g_engine->graphicsManager().getMessage(catalog, 4, noun, verb, kase, startSeq);
	debug("Talk: %s", str.c_str());
}

void Script::op_KQSound__play(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	g_engine->playSound(args[0]);
}

void Script::op_doPopup(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	trace_entry();
	auto str = g_engine->graphicsManager().getMessage(500, 99, 0, 0, getNumber(args[0]), 1);
	debug("Popup: %s", str.c_str());
}
} // namespace Kq8
