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

#ifndef KQ8_GUI_H
#define KQ8_GUI_H

#include "common/archive.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/palette.h"

namespace Kq8 {
class Bitmap;
class Font;

class Gui {
	enum class ControlType {
		kText,
		kBitmap,
		kButton,
		kGuiDialog,
		kBitmapDialog,
	};

	struct Control {
		ControlType _tag;
		Common::Rect _rect;
		uint32 _id;
		Common::String _font;
		Common::String _label;
		Common::String _bitmap;
		Font *_gfxFont = nullptr;
		Bitmap *_gfxBitmap = nullptr;
	};

	struct Dialog {
		ControlType _tag;
		Common::Rect _rect;
		uint32 _id;
		Common::Array<Control> _controls;
		Common::Array<Dialog> _dialogs;
		Common::String _bitmap;
		Bitmap *_gfxBitmap = nullptr;
	};

	Common::String _palette;
	Common::String _filename;
	Dialog _rootDialog;

public:
	Gui(const Common::String &filename, const Common::String &palette);
	~Gui();
	void prepareDialog(Graphics::Palette *palette, Dialog &dialog);
	void prepare();
	void drawDialog(const Dialog &dialog);
	void draw();

private:
	Dialog readDialog(Common::SeekableReadStream *stream, bool topLevel);
	Control readControl(Common::SeekableReadStream *stream);
};

} // namespace Kq8

#endif // KQ8_GUI_H
