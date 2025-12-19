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
#include "math/glmath.h"
#include "math/quat.h"

#include "graphics/opengl/context.h"
#include "graphics/opengl/shader.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/texture.h"

#include "kq8.h"
#include "kq8/bitmap.h"
#include "kq8/font.h"
#include "kq8/gfx_opengls.h"
#include "kq8/objects/camera.h"

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

	const char *mesh_attributes[] = {
		"position",
		"texcoord",
		nullptr,
	};
	_meshShader = OpenGL::Shader::fromFiles("kq8_mesh", mesh_attributes);

	const char *terrain_attributes[] = {
		"position",
		"texcoord",
		nullptr,
	};
	_terrain.shader = OpenGL::Shader::fromFiles("kq8_terrain", terrain_attributes);
	_terrain.shader->setUniform("tex", 0);
	_terrain.vbo = GL_INVALID_VALUE;

	const char *interior_attributes[] = {
		"position",
		"texcoord",
		nullptr,
	};
	_interiorShader = OpenGL::Shader::fromFiles("kq8_interior", interior_attributes);
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
	tex->setWrapMode(OpenGL::kWrapModeRepeat);
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

void GfxOpenGLS::loadInterior(Interior *interior) {
	struct InteriorVertex {
		Math::Vector3d _position;
		Math::Vector2d _texcoord;
	};
	const auto &surfaces = interior->surfaces();
	const auto &points = interior->points();
	const auto &texCoords = interior->texCoords();
	const auto &materials = interior->materials();

	Common::Array<Common::Array<uint32> > partitionedByMaterial;
	partitionedByMaterial.resize(materials.size());
	uint32 totalVertices = 0;

	for (int i = 0; i < surfaces.size(); ++i) {
		const auto &s = surfaces[i];
		totalVertices += 3 * (s._numVertices - 2);
		partitionedByMaterial[s._material].push_back(i);
	}

	Common::Array<InteriorVertex> vertices;
	vertices.reserve(totalVertices);
	Common::Array<InteriorSurface> interiorSurfaces;

	for (const auto &partition : partitionedByMaterial) {
		if (partition.empty()) {
			continue;
		}
		OpenGL::Texture *texture = _subTextures[materials[surfaces[partition[0]]._material]].texture;
		Math::Vector2d textureSize{float(texture->getWidth()), float(texture->getHeight())};

		uint32 numVertices = 0;
		for (auto idx : partition) {
			const auto &s = surfaces[idx];
			numVertices += 3 * (s._numVertices - 2);
			Common::Array<InteriorVertex> triangle;
			triangle.reserve(3);

			auto texScale = V2(int(s._texScaleX) + 1, int(s._texScaleY) + 1) / textureSize;
			auto texOffset = V2(s._texOffsetX, s._texOffsetY) / textureSize;

			for (uint32 i = s._vertIdx; i < s._vertIdx + s._numVertices; ++i) {
				const auto &iv = interior->vertices()[i];
				triangle.push_back({points[iv._pointIdx], texOffset + texScale * texCoords[iv._texCoordIdx]});
				if (triangle.size() == 3) {
					vertices.insert_at(vertices.size(), triangle);
					triangle.remove_at(1);
				}
			}
		}
		interiorSurfaces.push_back(InteriorSurface{texture, numVertices});
	}

	auto vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data());

	OpenGL::Shader *shader = _interiorShader->clone();
	shader->enableVertexAttribute("position", vbo, 3, GL_FLOAT, false, sizeof(InteriorVertex), offsetof(InteriorVertex, _position));
	shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, false, sizeof(InteriorVertex), offsetof(InteriorVertex, _texcoord));
	_interiors[interior] = {shader, vbo, Common::move(interiorSurfaces)};
}

void GfxOpenGLS::loadTerrain(Terrain *terrain) {
	struct TerrainVertex {
		Math::Vector3d _position;
		Math::Vector2d _texcoord;
	};
	if (_terrain.vbo != GL_INVALID_VALUE) {
		glDeleteBuffers(1, &_terrain.vbo);
		_terrain.partitions.clear();
	}

	using Coord = Common::Pair<uint8, uint8>;
	Common::Array<Common::Array<Coord> > partitionedByMaterial;
	partitionedByMaterial.resize(256);
	for (uint8 r = 0; r < terrain->height(); ++r) {
		for (uint8 c = 0; c < terrain->width(); ++c) {
			auto &tile = terrain->tileAt(c, r);
			partitionedByMaterial[tile.material].emplace_back(Coord{c, r});
		}
	}

	Common::Array<TerrainVertex> vertices;
	auto materialIt = terrain->materials().begin();
	for (const auto &partition : partitionedByMaterial) {
		if (partition.empty())
			continue;

		auto *bitmap = *materialIt;
		materialIt++;
		_terrain.partitions.emplace_back(partition.size() * 6, _subTextures[bitmap].texture);
		for (const auto &coord : partition) {
			int c = coord.first;
			int r = coord.second;
			const auto &tile = terrain->tileAt(c, r);
			vertices.emplace_back(TerrainVertex{V3(c + 0, r + 0, tile.heights[0]), V2(0, 0)});
			vertices.emplace_back(TerrainVertex{V3(c + 0, r + 1, tile.heights[2]), V2(0, 1)});
			vertices.emplace_back(TerrainVertex{V3(c + 1, r + 0, tile.heights[1]), V2(1, 0)});
			vertices.emplace_back(TerrainVertex{V3(c + 1, r + 0, tile.heights[1]), V2(1, 0)});
			vertices.emplace_back(TerrainVertex{V3(c + 0, r + 1, tile.heights[2]), V2(0, 1)});
			vertices.emplace_back(TerrainVertex{V3(c + 1, r + 1, tile.heights[3]), V2(1, 1)});
		}
	}

	_terrain.vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
	_terrain.shader->enableVertexAttribute("position", _terrain.vbo, 3, GL_FLOAT, false, sizeof(vertices[0]), offsetof(TerrainVertex, _position));
	_terrain.shader->enableVertexAttribute("texcoord", _terrain.vbo, 2, GL_FLOAT, false, sizeof(vertices[0]), offsetof(TerrainVertex, _texcoord));
}

struct MeshVBOElement {
	Math::Vector3d _position;
	Math::Vector2d _texcoord;
};

bool GfxOpenGLS::MeshPartition::operator<(const GfxOpenGLS::MeshPartition &rhs) const {
	if (_mesh < rhs._mesh)
		return true;
	if (rhs._mesh < _mesh)
		return false;
	return _frame < rhs._frame;
}

void GfxOpenGLS::loadShape(Shape *shape) {
	Common::Array<MeshVBOElement> vertices;
	uint32 totalVboSize = 0;
	for (const auto &mesh : shape->_meshes) {
		totalVboSize += mesh._frames.size() * mesh._faces.size() * 3;
	}
	vertices.reserve(totalVboSize);

	Common::Array<MeshPartition> partitions;
	uint16 currentMesh = 0;
	for (const auto &mesh : shape->_meshes) {
		uint16 currentFrame = 0;
		for (const auto &frame : mesh._frames) {
			auto currentMaterial = mesh._faces[0]._material;
			auto *initialTexture = _subTextures[shape->_materials[currentMaterial]].texture;
			partitions.push_back(MeshPartition{currentMesh, currentFrame, vertices.size(), 0, initialTexture});

			for (const auto &face : mesh._faces) {
				if (face._material != currentMaterial) {
					currentMaterial = face._material;
					auto *texture = _subTextures[shape->_materials[currentMaterial]].texture;
					partitions.push_back(MeshPartition{currentMesh, currentFrame, vertices.size(), 0, texture});
				}

				partitions.back()._numVertices += 3;
				for (int i = 0; i < 3; ++i) {
					const auto packedVertex = &mesh._packedVertices[4 * (frame._firstVertex + face._verts[i])];
					const auto &texcoord = mesh._texcoords[face._texcoords[i]];
					vertices.push_back(MeshVBOElement{
						Math::Vector3d{float(packedVertex[0]), float(packedVertex[1]), float(packedVertex[2])} / 255.0,
						Math::Vector2d{texcoord._u, texcoord._v}});
				}
			}
			currentFrame++;
		}
		currentMesh++;
	}

	auto vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
	auto shader = _meshShader->clone();
	shader->enableVertexAttribute("position", vbo, 3, GL_FLOAT, false, sizeof(MeshVBOElement), offsetof(MeshVBOElement, _position));
	shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, false, sizeof(MeshVBOElement), offsetof(MeshVBOElement, _texcoord));

	_shapes[shape] = {shader, Common::move(partitions)};
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

void GfxOpenGLS::drawTerrain(Terrain *terrain) {
	// glDisable(GL_CULL_FACE);
	_terrain.shader->use();
	_terrain.shader->setUniform("projectionMatrix", _projectionMatrix);
	_terrain.shader->setUniformTransposed("viewMatrix", _viewMatrix);
	Math::Matrix4 modelMatrix;
	modelMatrix.setToIdentity();
	modelMatrix(0, 0) = terrain->groundScale();
	modelMatrix(1, 1) = terrain->groundScale();
	modelMatrix(2, 2) = terrain->heightScale();
	_terrain.shader->setUniform("tex", 0);
	_terrain.shader->setUniform("modelMatrix", modelMatrix);
	int offset = 0;
	for (const auto &partition : _terrain.partitions) {
		if (partition.second) {
			partition.second->bind();
			GL_CALL(glDrawArrays(GL_TRIANGLES, offset, partition.first));
		}
		offset += partition.first;
	}
}

void GfxOpenGLS::drawNode(Shape *shape, const Math::Matrix4 &objectTransform, const Math::Matrix4 &nodeTransform, uint16 mesh, uint16 frame) {
	auto &shapeInfo = _shapes[shape];
	auto *shader = shapeInfo.first;
	auto &meshPartitions = shapeInfo.second;
	shader->use();
	shader->setUniform("projectionMatrix", _projectionMatrix);
	shader->setUniformTransposed("viewMatrix", _viewMatrix);
	shader->setUniform("tex", 0);
	shader->setUniformTransposed("modelMatrix", objectTransform);
	shader->setUniformTransposed("nodeTransform", nodeTransform);
	shader->setUniformTransposed("frameTransform", shape->_meshes[mesh]._frames[frame]._transform);

	auto partition = Common::lowerBound(meshPartitions.begin(), meshPartitions.end(), MeshPartition{mesh, frame});
	while (partition != meshPartitions.end() && partition->_mesh == mesh && partition->_frame == frame) {
		partition->_texture->bind();
		GL_CALL(glDrawArrays(GL_TRIANGLES, partition->_firstVertex, partition->_numVertices));
		partition++;
	}
}

void GfxOpenGLS::drawInterior(Interior *interior) {
	auto &interiorInfo = _interiors[interior];
	auto shader = interiorInfo._shader;
	shader->use();
	shader->setUniform("projectionMatrix", _projectionMatrix);
	shader->setUniformTransposed("viewMatrix", _viewMatrix);
	shader->setUniformTransposed("modelMatrix", interior->getTransform());
	shader->setUniform("tex", 0);

	uint32 offset = 0;
	for (const auto &is : interiorInfo._interiorSurfaces) {
		is._texture->bind();
		GL_CALL(glDrawArrays(GL_TRIANGLES, offset, is._numVertices));
		offset += is._numVertices;
	}
}

void GfxOpenGLS::setupCamera() {
	_projectionMatrix = Math::makeFrustumMatrix(-320, 320, 240, -240, 256, 1000000);
	Camera *camera = (Camera *)g_engine->world()->findObject("KQCamera");
	const auto &eye = camera->pos();
	const auto &rotation = camera->rot();
	auto direction = Math::Vector3d{0, 0, -1};
	auto up = Math::Vector3d{0, -1, 0};
	auto q = Math::Matrix4{
		Math::Angle::fromRadians(rotation.z()),
		Math::Angle::fromRadians(rotation.x()),
		Math::Angle::fromRadians(rotation.y()),
		Math::EO_YZX};
	q.transform(&direction, false);
	q.transform(&up, false);
	auto undoCamera = Math::Matrix4{};
	undoCamera.setToIdentity();
	undoCamera.setPosition(-eye);

	auto flipYZ = Math::Matrix4{};
	flipYZ.setToIdentity();
	flipYZ(1, 1) = flipYZ(2, 2) = 0;
	flipYZ(2, 1) = -1;
	flipYZ(1, 2) = 1;

	_viewMatrix = Math::makeLookAtMatrix(Math::Vector3d{}, direction, up) * flipYZ * undoCamera;

	glEnable(GL_DEPTH_TEST);
}

} // namespace Kq8
#endif // USE_OPENGL_SHADERS
