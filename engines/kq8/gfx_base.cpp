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

#include "gfx_base.h"

#include "kq8/shape.h"

namespace Kq8 {
void GfxBase::drawShape(Shape *shape, const Math::Matrix4 &transform, int sequence) {
	const auto &seq = shape->_sequences[sequence];
	auto ss = seq._subsequenceIndex;
	const auto &sseq = shape->_subSequences[ss];
	const auto &seqTransform = seq._transform;
	for (int nodeIdx = sseq._nodeIndex; nodeIdx < sseq._nodeIndex + sseq._nodeCount; ++nodeIdx) {
		const auto &node = shape->_nodes[nodeIdx];
		drawNode(shape, transform, seqTransform * node._transform, node._mesh, node._frame);
	}
}
} // namespace Kq8