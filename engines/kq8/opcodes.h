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

// list of opcodes

#ifndef OPCODE
#error "Missing definition of OPCODE"
#endif

#ifndef OPCODE2
#error "Missing definition of OPCODE2"
#endif

OPCODE(alias);
OPCODE(doPopup);
OPCODE(echo);
OPCODE(getEndLoop);
OPCODE(loadKQ);
OPCODE(lockResource);
OPCODE(missing);
OPCODE(move);
OPCODE(purgeResource);
OPCODE(sendEvent);
OPCODE(set);
OPCODE(setcat);
OPCODE(setLoadProgress);

OPCODE2(KQObject::setScript, KQObject__setScript);
OPCODE2(KQMonster::setState, KQMonster__setState);