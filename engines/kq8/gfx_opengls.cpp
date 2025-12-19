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

struct TextVBOElement {
	TextVBOElement() : position{0, 0}, texcoord{0, 0} {};
	TextVBOElement(const Math::Vector2d &position, const Math::Vector2d &texcoord)
		: position{position},
		  texcoord{texcoord} {}
	Math::Vector2d position;
	Math::Vector2d texcoord;
};

template<class T1, class T2>
static inline Math::Vector2d V2(T1 x, T2 y) {
	return {static_cast<float>(x), static_cast<float>(y)};
}

template<class T1, class T2, class T3>
static inline Math::Vector3d V3(T1 x, T2 y, T3 z) {
	return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
}

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

	const char *text_attributes[] = {
		"position",
		"texcoord",
		nullptr,
	};
	_textVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(TextVBOElement) * 1200, nullptr, GL_STREAM_DRAW);
	_textShader = OpenGL::Shader::fromFiles("kq8_text", text_attributes);
	_textShader->enableVertexAttribute("position", _textVBO, 2, GL_FLOAT, false, sizeof(TextVBOElement), offsetof(TextVBOElement, position));
	_textShader->enableVertexAttribute("texcoord", _textVBO, 2, GL_FLOAT, false, sizeof(TextVBOElement), offsetof(TextVBOElement, texcoord));
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

void GfxOpenGLS::loadBitmapLoose(Bitmap *bmp) {
	auto rect = Common::Rect{bmp->surface()->w, bmp->surface()->h};
	auto tex = new OpenGL::Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	tex->setSize(rect.width(), rect.height());
	tex->bind();
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0,
						 GL_RGB,
						 rect.width(), rect.height(),
						 0,
						 GL_RGB, GL_UNSIGNED_BYTE, bmp->surface()->getPixels()));
	_subTextures[bmp] = SubTexture{tex, rect};
}

void GfxOpenGLS::loadFont(Font *font) {
	auto tex = new OpenGL::Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, false);
	auto atlas = font->atlas();
	tex->setSize(atlas->w, atlas->h);
	tex->create();

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas->w, atlas->h, GL_RGB, GL_UNSIGNED_BYTE, atlas->getPixels());

	Common::HashMap<unsigned char, SubTexture> subTextures;
	for (auto kv : font->charMap()) {
		subTextures[kv._key] = SubTexture{tex, kv._value};
	}
	_fonts[font] = subTextures;
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

void GfxOpenGLS::drawText(const Font *font, const Common::String &label, const Common::Rect &rect) {
	Common::Array<TextVBOElement> vertices;
	vertices.reserve(label.size() * 6);

	Math::Vector2d point = V2(rect.left, rect.top);
	auto &atlas = _fonts[font];
	float factor = float(rect.height()) / font->boundingBox().height();
	OpenGL::Texture *texture = nullptr;
	for (auto c : label) {
		if (atlas.contains(c)) {
			if (!texture)
				texture = atlas[c].texture;
			const auto &r = atlas[c].rect;
			auto charW = r.width() * factor;
			vertices.emplace_back(point + V2(0, 0), V2(r.left, r.top) + V2(0.5, 0.5));
			vertices.emplace_back(point + V2(0, rect.height() - 1), V2(r.left, r.bottom) + V2(0.5, -0.5));
			vertices.emplace_back(point + V2(charW - 1, 0), V2(r.right, r.top) + V2(-0.5, 0.5));
			vertices.emplace_back(point + V2(charW - 1, rect.height() - 1), V2(r.right, r.bottom) + V2(-0.5, -0.5));
			vertices.emplace_back(point + V2(charW - 1, 0), V2(r.right, r.top) + V2(-0.5, 0.5));
			vertices.emplace_back(point + V2(0, rect.height() - 1), V2(r.left, r.bottom) + V2(0.5, -0.5));
			point += V2(charW + 1, 0);
		}
	}

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _textVBO));
	GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vertices[0]), vertices.data()));
	_textShader->use();
	texture->bind();
	_textShader->setUniform("tex", 0);
	_textShader->setUniform("texSizeWH", V2(texture->getWidth(), texture->getHeight()));
	_textShader->setUniform("color", Math::Vector4d{1, 1, 1, 1});
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));
}

} // namespace Kq8
#endif // USE_OPENGL_SHADERS
