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

#ifndef KQ8_GFXBASE_H
#define KQ8_GFXBASE_H

#include "common/ptr.h"
#include "common/rect.h"
#include "objects/object.h"
#include "shape.h"

namespace Kq8 {

class Bitmap;
class Font;
class Terrain;

class GfxBase {
public:
	virtual ~GfxBase() {}

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void loadBitmap(Bitmap *bmp) = 0;
	virtual void loadBitmapLoose(Bitmap *bmp) = 0;
	virtual void loadFont(Font *font) = 0;
	virtual void loadTerrain(Terrain *terrain) = 0;
	virtual void loadShape(Shape *shape) = 0;
	virtual void drawBitmap(const Bitmap *bmp, const Common::Rect &rect) = 0;
	virtual void drawText(const Font *font, const Common::String &label, const Common::Rect &rect) = 0;
	virtual void drawTerrain(Terrain *terrain) = 0;
	virtual void setupCamera() = 0;
	virtual void drawShape(Shape *shape, int sequence, const Math::Matrix4 &transform);
	virtual void drawNode(Shape *shape, const Math::Matrix4 &objectTransform, const Math::Matrix4 &nodeTransform, uint16 mesh, uint16 frame) = 0;
};

} // namespace Kq8

#endif // KQ8_GFXBASE_H
