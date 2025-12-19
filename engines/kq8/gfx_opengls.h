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

#ifndef KQ8_GFX_OPENGLS_H
#define KQ8_GFX_OPENGLS_H
#include "graphics/opengl/shader.h"

#ifdef USE_OPENGL_SHADERS

#include "common/array.h"
#include "common/hash-ptr.h"
#include "common/hashmap.h"
#include "common/rect.h"

#include "kq8/gfx_base.h"
#include "kq8/texture_packer.h"

namespace Kq8 {

class GfxOpenGLS : public GfxBase {
	Common::Array<TexturePacker> _texturePackers;
	Common::HashMap<const Bitmap *, SubTexture> _subTextures;
	Common::HashMap<const Font *, Common::HashMap<unsigned char, SubTexture> > _fonts;
	OpenGL::Shader *_bitmapShader;
	GLuint _bitmapVBO;
	OpenGL::Shader *_textShader;
	GLuint _textVBO;
	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _viewMatrix;

public:
	GfxOpenGLS();
	void clearScreen() override;
	void flipBuffer() override;
	void loadBitmap(Bitmap *bmp) override;
	void loadBitmapLoose(Bitmap *bmp) override;
	void loadFont(Font *font) override;
	void drawBitmap(const Bitmap *bmp, const Common::Rect &rect) override;
	void drawText(const Font *font, const Common::String &label, const Common::Rect &rect) override;
	void setupCamera() override;
};

} // namespace Kq8
#endif // USE_OPENGL_SHADERS
#endif // KQ8_GFX_OPENGLS_H
