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

#ifndef KQ8_SCRIPT_H
#define KQ8_SCRIPT_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"

#include "kq8/script.h"

namespace Kq8 {
struct Expr;
struct LineExpr;
struct Block {
	Common::Array<Expr *> _body;
	~Block();
};
class Script {
private:
	Common::String _name;
	Block _body;

public:
	Script(const Common::String &name);
	using Environment = Common::HashMap<Common::String, Common::String>;
	using Args = Common::Array<Common::String>;

	void evaluate(Script::Environment &env, const Script::Args &args);
	Common::String evaluateExpr(Script::Environment &env, const Script::Args &args, const Common::String &string);
	void evaluate(Script::Environment &env, const Script::Args &args, const Block &block);

	using OpcodeFn = void (Script::*)(Script::Environment &, const Script::Args &, LineExpr *);
	static const Common::HashMap<Common::String, OpcodeFn> &getOpcodes();

#define OPCODE(x) void op_##x(Script::Environment &, const Script::Args &, LineExpr *);
#define OPCODE2(x, y) void op_##y(Script::Environment &, const Script::Args &, LineExpr *);
#include "kq8/opcodes.h"

#undef OPCODE
#undef OPCODE2
};

} // namespace Kq8

#endif // SCRIPT_H
