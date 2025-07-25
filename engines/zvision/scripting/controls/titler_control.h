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

#ifndef ZVISION_TITLER_CONTROL_H
#define ZVISION_TITLER_CONTROL_H

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "zvision/scripting/control.h"

namespace ZVision {

// Only used in Zork Nemesis, handles the death screen with the Restore/Exit buttons
class TitlerControl : public Control {
public:
	TitlerControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~TitlerControl() override;

	void setString(int strLine);

private:

	Common::Array< Common::String > _strings;
	Common::Rect _rectangle;
	int16 _curString;
	Graphics::Surface *_surface;

	void readStringsFile(const Common::Path &fileName);
};

} // End of namespace ZVision

#endif
