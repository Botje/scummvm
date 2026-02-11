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
#include "common/util.h"

#include "kq8/gfx_base.h"
#include "kq8/texture_packer.h"

namespace Kq8 {

class GfxOpenGLS : public GfxBase {
	struct MeshPartition {
		uint16 _mesh;
		uint16 _frame;
		uint32 _firstVertex;
		uint32 _numVertices;
		OpenGL::Texture *_texture;
		bool operator<(const MeshPartition &rhs) const;
	};
	using ShapeInformation = Common::Pair<OpenGL::Shader *, Common::Array<MeshPartition> >;
	struct InteriorSurface {
		OpenGL::Texture *_texture;
		uint32 _numVertices;
	};
	struct InteriorInformation {
		OpenGL::Shader *_shader;
		GLuint _vbo;
		Common::Array<InteriorSurface> _interiorSurfaces;
	};

	Common::Array<TexturePacker> _texturePackers;
	Common::HashMap<const Bitmap *, SubTexture> _subTextures;
	Common::HashMap<const Font *, Common::HashMap<unsigned char, SubTexture> > _fonts;
	Common::HashMap<const Shape *, ShapeInformation> _shapes;
	Common::HashMap<const Interior *, InteriorInformation> _interiors;
	OpenGL::Shader *_bitmapShader;
	GLuint _bitmapVBO;
	OpenGL::Shader *_textShader;
	GLuint _textVBO;
	OpenGL::Shader *_meshShader;
	struct {
		GLuint vbo = GL_INVALID_VALUE;
		GLuint ebo = GL_INVALID_VALUE;
		OpenGL::Shader *shader;
		// The EBO contains triangles partitioned by material.
		// This field contains number of vertices + material.
		Common::Array<Common::Pair<uint32, OpenGL::Texture *> > partitions;
	} _terrain;
	OpenGL::Shader *_interiorShader;
	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _viewMatrix;

	Common::Array<const Object *> _mousePickIndices;

public:
	GfxOpenGLS();
	void clearScreen() override;
	void flipBuffer() override;
	void loadBitmap(Bitmap *bmp) override;
	void loadBitmapLoose(Bitmap *bmp) override;
	void loadFont(Font *font) override;
	void loadInterior(Interior *interior) override;
	void loadTerrain(Terrain *terrain) override;
	void drawShape(const Object *object, Shape *shape, const Math::Matrix4 &transform, int sequence) override;
	void loadShape(Shape *shape) override;
	void drawBitmap(const Bitmap *bmp, const Common::Rect &rect) override;
	void drawText(const Font *font, const Common::String &label, const Common::Rect &rect) override;
	void drawTerrain(Terrain *terrain) override;
	void drawNode(Shape *shape, const Math::Matrix4 &objectTransform, const Math::Matrix4 &nodeTransform, uint16 mesh, uint16 frame) override;
	void drawInterior(Interior *interior) override;
	void setupCamera() override;
	void setupOverlay() override;
	const Object *mousePick(const Common::Point &point) override;
};

} // namespace Kq8
#endif // USE_OPENGL_SHADERS
#endif // KQ8_GFX_OPENGLS_H
