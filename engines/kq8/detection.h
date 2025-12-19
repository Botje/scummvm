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

#ifndef KQ8_DETECTION_H
#define KQ8_DETECTION_H

#include "engines/advancedDetector.h"

namespace Kq8 {

enum Kq8DebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,
};

extern const PlainGameDescriptor kq8Games[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Kq8

class Kq8MetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	Kq8MetaEngineDetection();
	~Kq8MetaEngineDetection() override {}

	const char *getName() const override {
		return "kq8";
	}

	const char *getEngineName() const override {
		return "Kq8";
	}

	const char *getOriginalCopyright() const override {
		return "Kq8 (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // KQ8_DETECTION_H
