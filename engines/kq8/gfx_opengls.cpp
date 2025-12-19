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

#ifdef USE_OPENGL_SHADERS

#include "common/hash-ptr.h"
#include "common/system.h"

#include "graphics/opengl/context.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#include "kq8/bitmap.h"
#include "kq8/font.h"
#include "kq8/gfx_opengls.h"

#include "graphics/opengl/shader.h"

namespace Kq8 {

GfxOpenGLS::GfxOpenGLS() {
	const char *bitmap_attributes[] = {
		"position",
		"texcoord",
		nullptr,
	};
	_bitmapShader = OpenGL::Shader::fromFiles("kq8_bitmap", bitmap_attributes);
	float bitmapCoords[][4] = {
		// X Y U V
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 0.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
	};
	_bitmapVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(bitmapCoords), bitmapCoords);
	_bitmapShader->enableVertexAttribute("position", _bitmapVBO, 2, GL_FLOAT, true, sizeof(*bitmapCoords), 0);
	_bitmapShader->enableVertexAttribute("texcoord", _bitmapVBO, 2, GL_FLOAT, true, sizeof(*bitmapCoords), 2 * sizeof(float));
	glEnable(GL_TEXTURE_2D);
}

void GfxOpenGLS::clearScreen() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void GfxOpenGLS::flipBuffer() {
	g_system->updateScreen();
}
void GfxOpenGLS::loadBitmap(Bitmap *bmp) {
	Common::Rect piece{bmp->surface()->w, bmp->surface()->h};
	Common::Rect location;

	for (auto &packer : _texturePackers) {
		if (packer.assign(piece, location)) {
			_subTextures[bmp] = SubTexture{packer.texture, location, bmp};
			return;
		}
	}

	auto tex = new OpenGL::Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	tex->setSize(1024, 512);
	_texturePackers.emplace_back(tex);
	_texturePackers.back().assign(piece, location);
	_subTextures[bmp] = SubTexture{tex, location, bmp};
}

void GfxOpenGLS::loadFont(Font *font) {
	auto tex = new OpenGL::Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, false);
	auto atlas = font->atlas();
	tex->setSize(atlas->w, atlas->h);
	tex->create();

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->w, atlas->h, GL_RGB, GL_UNSIGNED_BYTE, atlas->getPixels());

	_fonts[font] =
}

void GfxOpenGLS::drawBitmap(const Bitmap *bmp, const Common::Rect &rect) {
	auto &subTexture = _subTextures[bmp];
	_bitmapShader->use();
	subTexture.texture->bind();
	_bitmapShader->setUniform("verOffsetXY", Math::Vector2d{static_cast<float>(rect.left), static_cast<float>(rect.top)});
	_bitmapShader->setUniform("verSizeWH", Math::Vector2d{static_cast<float>(rect.width()), static_cast<float>(rect.height())});
	_bitmapShader->setUniform("texOffsetXY", subTexture.getTexOffset());
	_bitmapShader->setUniform("texSizeWH", subTexture.getTexSize());
	_bitmapShader->setUniform("tex", 0);
	_bitmapShader->setUniform("color", Math::Vector4d{1, 1, 1, 1});
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

} // namespace Kq8
#endif // USE_OPENGL_SHADERS
