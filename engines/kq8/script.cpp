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

#include "kq8/kq8.h"
#include "kq8/kq_file.h"
#include "kq8/script.h"

namespace Kq8 {

static const Common::String EMPTY_TOKEN = "<EMPTY>";
using Tokens = Common::Array<Common::String>;

static Common::String joinArgs(const Script::Args &args) {
	Common::String result;
	for (auto &a : args) {
		result += a;
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

		if (*pos_ == '"') {
			pos_++;
			auto start = pos_;
			while (pos_ != str_.end() && *pos_ != '"') {
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
	LineExpr(const Tokens &line) : line_(line) {}
	LineExpr(Tokens &&line) : line_(line) {}
	Tokens line_;
	const Common::String &tokenAt(uint i) const { return line_[i]; }
	float numberAt(uint i) const {
		float ret;
		sscanf(tokenAt(i).c_str(), "%f", &ret);
		return ret;
	}
};

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
	: _name(name), _line(1) {
	auto stream = SearchMan.createReadStreamForMember(Common::Path{name});
	if (!stream) {
		error("Cannot load '%s'", name.c_str());
	}
	auto blocks = Common::Stack<Block *>{};
	blocks.push(&_body);
	while (!stream->eos()) {
		_line++;
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
			auto condition = LineExpr{tokenizer.rest()};
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
			blocks.top()->_body.push_back(new LineExpr{tokenizer.rest()});
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
			if (first.hasSuffix(".cs")) {
				Script::Args cs_args;
				for (auto it = lineExpr->line_.begin() + 1; it != lineExpr->line_.end(); it++) {
					cs_args.emplace_back(evaluateExpr(env, args, *it));
				}
				Script{first}.evaluate(env, cs_args);
			} else {
				auto opcode = getOpcodes().getValOrDefault(first);
				if (!opcode) {
					opcode = &Script::op_missing;
				}
				(this->*opcode)(env, args, lineExpr);
			}
		}
	}
}
Common::String Script::evaluateExpr(Script::Environment &env, const Script::Args &args, const Common::String &string) {
	if (string.hasPrefix("$")) {
		if (Common::isDigit(string[1])) {
			return args[string[1] - '1']; // $1 is the first argument
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
#include "kq8/opcodes.h"

#undef OPCODE
		opcodes.erase("missing");
	}
	return opcodes;
}

void Script::op_missing(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	Common::String fullLine;
	for (const auto &token : expr->line_) {
		fullLine += token;
		fullLine += " ";
	}

	warning("Opcode missing: %s", fullLine.c_str());
}

void Script::op_move(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	enum MoveFlag {
		kMoveAbsRotAbs = 0,
		kMoveRelRotAbs = 1,
		kMoveAbsRotRel = 2,
		kMoveRelRotRel = 3,

	};
	auto who = expr->tokenAt(1);
	MoveFlag flag = (MoveFlag)expr->numberAt(2);
	auto x = expr->numberAt(3);
	auto y = expr->numberAt(4);
	auto z = expr->numberAt(5);

	auto *obj = g_engine->world()->findObject(who);
	if (!obj) {
		warning("move: Could not find object %s", who.c_str());
		return;
	}

	Math::Vector3d newPos = Math::Vector3d(x, y, z);
	if (flag == kMoveRelRotAbs || flag == kMoveRelRotRel) {
		newPos += obj->pos();
	}

	obj->moveTo(newPos);

	if (expr->line_.size() == 9) {
		auto lx = expr->numberAt(6);
		auto ly = expr->numberAt(7);
		auto lz = expr->numberAt(8);
		Math::Vector3d newRot = Math::Vector3d(lx, ly, lz);
		if (flag == kMoveRelRotRel || flag == kMoveAbsRotRel) {
			newRot += obj->rot();
		}

		obj->setRotation(newRot);
	}
}

void Script::op_set(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	auto val = evaluateExpr(env, args, expr->tokenAt(2));
	env.setVal(expr->tokenAt(1), val);
}

void Script::op_loadKQ(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	KQFile kqFile;
	auto file = expr->tokenAt(1);
	auto pos = file.find('/');
	if (pos != Common::String::npos) {
		file = file.substr(pos + 1);
	}
	auto stream = Common::ScopedPtr<Common::SeekableReadStream>{SearchMan.createReadStreamForMember(Common::Path{file})};

	if (!stream) {
		warning("Could not loadKQ '%s'", file.c_str());
		return;
	}
	kqFile.loadFromStream(*stream);
	auto &section = kqFile.getSections().front();
	auto klass = section.getKey("classType")->value;
	debug("loaded KQ %s, classType=%s", file.c_str(), klass.c_str());

	auto *obj = g_engine->objectFactory().load(klass, kqFile);
	if (obj && expr->line_.size() == 9) {
		auto name = expr->tokenAt(2);
		if (name != "same") {
			obj->setName(name);
		}

		auto x = expr->numberAt(3);
		auto y = expr->numberAt(4);
		auto z = expr->numberAt(5);
		obj->moveTo(Math::Vector3d{x, y, z});

		auto rx = expr->numberAt(6);
		auto ry = expr->numberAt(7);
		auto rz = expr->numberAt(8);
		obj->setRotation(Math::Vector3d{rx, ry, rz});
	}
}
void Script::op_lockResource(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	// Do nothing
}
void Script::op_purgeResource(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	// Do nothing
}

void Script::op_setcat(Script::Environment &env, const Script::Args &args, LineExpr *expr) {
	auto variable = expr->tokenAt(1);
	Common::String value;
	for (auto it = expr->line_.begin() + 2; it != expr->line_.end(); ++it) {
		value += evaluateExpr(env, args, *it);
	}

	env.setVal(variable, value);
}

void Script::op_setLoadProgress(Script::Environment &, const Script::Args &, LineExpr *) {
	// Do nothing
}

void Script::op_echo(Script::Environment &, const Script::Args &args, LineExpr *expr) {
	debug("%s", joinArgs(args).c_str());
}

void Script::op_alias(Script::Environment &, const Script::Args &, LineExpr *) {
	// Do nothing
}
} // namespace Kq8