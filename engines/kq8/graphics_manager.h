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

#ifndef KQ8_GRAPHICSMANAGER_H
#define KQ8_GRAPHICSMANAGER_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/palette.h"
#include "kq8/animation_loop_list.h"
#include "kq8/gui.h"
#include "kq8/msg_file.h"
#include "kq8/objects/interior.h"
#include "kq8/objects/terrain.h"

namespace Kq8 {

class Bitmap;
class Font;
class Interior;
class Shape;

class GraphicsManager {

private:
	Common::HashMap<Common::String, Graphics::Palette *> _palettes;
	Common::HashMap<Common::String, Kq8::Font *> _fonts;
	Common::HashMap<Common::String, Kq8::Bitmap *> _bitmaps;
	Common::HashMap<Common::String, Kq8::Shape *> _shapes;
	Common::HashMap<Common::String, Kq8::AnimationLoopList *> _animationLoops;
	Common::HashMap<uint16, MsgFile> _msgFiles;

public:
	Graphics::Palette *getPalette(const Common::String &p);
	~GraphicsManager();

	enum class BitmapPacking {
		kPacked,
		kLoose,
	};

	Font *loadFont(const Common::String &name, const Graphics::Palette *palette);
	Bitmap *loadBitmap(const Common::String &name, const Graphics::Palette *palette, BitmapPacking packing = BitmapPacking::kPacked);
	void loadTerrain(Terrain *terrain);
	Shape *loadshape(const Common::String &name);
	void loadInterior(Interior *interior);
	AnimationLoopList *loadAnimationLoopList(const Common::String &name);

	void drawBitmap(const Bitmap *bitmap, const Common::Rect &rect);
	void drawText(const Font *font, const Common::String &label, const Common::Rect &position);
	void drawText(const Font *font, const Common::String &label, const Common::Point &position);

	// TODO: this is cause for renaming GraphicsManager to ResourceManager
	const Common::String getMessage(int catalog, uint8 talker, uint8 noun, uint8 verb, uint8 kase, uint8 sequence);
};

} // namespace Kq8

#endif // GRAPHICS_H
