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

#include "common/debug.h"
#include "common/memstream.h"

#include "teenagent/objects.h"
#include "teenagent/resources.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

void Rect::load(byte *src) {
	_base = src;
	Common::MemoryReadStream ins(src, 8);
	left = ins.readUint16LE();
	top = ins.readUint16LE();
	right = ins.readUint16LE();
	bottom = ins.readUint16LE();
}

void Rect::save() const {
	assert(_base != NULL);
	Common::MemoryWriteStream outs(_base, 8);
	outs.writeUint16LE(left);
	outs.writeUint16LE(top);
	outs.writeUint16LE(right);
	outs.writeUint16LE(bottom);
}

void Rect::render(Graphics::Surface *surface, uint8 color) const {
	surface->hLine(left, bottom, right, color);
	surface->vLine(left, bottom, top, color);
	surface->hLine(left, top, right, color);
	surface->vLine(right, bottom, top, color);
}

void Object::load(byte *src, byte sceneId) {
	_base = src;

	id = *src++;

	rect.load(src);
	src += 8;
	actorRect.load(src);
	src += 8;

	actorOrientation = *src++;
	enabled = *src++;
	name = (const char *)src;
	_nameSize = name.size() + 1;
	src += _nameSize;

	bool hasRealName = (sceneId == 6 && id == 4) ||
						(sceneId == 23 && id == 2) ||
						(sceneId == 20 && id == 13) ||
						(sceneId == 32 && id == 1);
	// Skip free space (if any) made for objects that have newName
	if (hasRealName) {
		while (*src == 0)
			src++;
	}

	description = parseDescription((const char *)src);

	if (hasRealName) {
		src += description.size() + 2;
		_realName = (const char *)src;
	}
}

void Object::save() const {
	assert(_base != NULL);

	rect.save();
	actorRect.save();
	_base[17] = actorOrientation;
	_base[18] = enabled;
}

void Object::setRealName() {
	assert(_base != 0);
	Common::strcpy_s((char *)(_base + 19), _nameSize, _realName.c_str());
	name = _realName;
}

void Object::dump(int level) const {
	debugC(level, kDebugObject, "object: %u %u [%u,%u,%u,%u], actor: [%u,%u,%u,%u], orientation: %u, name: %s", id, enabled,
	      rect.left, rect.top, rect.right, rect.bottom,
	      actorRect.left, actorRect.top, actorRect.right, actorRect.bottom,
	      actorOrientation, name.c_str()
	     );
}

Common::String Object::parseDescription(const char *desc) {
	if (*desc == 0)
		return Common::String();

	Common::String result;

	while (*desc != 1 && *desc != 0) {
		Common::String line;
		while (*desc != 1 && *desc != 0) {
			debugC(2, kDebugObject, "%02x ", *desc);
			line += *desc++;
		}

		if (line.empty())
			break;

		++desc;
		result += line;
		result += '\n';
	}
	if (!result.empty())
		result.deleteLastChar();
	else
		result = g_engine->parseMessage(g_engine->res->getMessageAddr(kCoolMsg));
	return result;
}

void InventoryObject::load(byte *src) {
	_base = src;
	id = *src++;
	animated = *src++;
	name = (const char *)src;
	description = Object::parseDescription((const char *)src);
}

void UseHotspot::load(byte *src) {
	Common::MemoryReadStream in(src, 9);
	inventoryId = in.readByte();
	objectId = in.readByte();
	orientation = in.readByte();
	actorX = in.readUint16LE();
	actorY = in.readUint16LE();
	callback = in.readUint16LE();
}

void UseHotspot::dump(int level) const {
	debugC(level, kDebugObject,
	      "hotspot: invId: %02x, objId: %02x, orientation: %02x, actor position: (%d,%d), callback: %04x",
	      inventoryId, objectId, orientation, actorX, actorY, callback
	     );
}

void Walkbox::dump(int level) const {
	debugC(level, kDebugObject, "walkbox %02x %02x [%d, %d, %d, %d] top: %u, right: %u, bottom: %u, left: %u",
	      type, orientation,
	      rect.left, rect.top, rect.right, rect.bottom,
	      sideHint[0], sideHint[1], sideHint[2], sideHint[3]);
}

void Walkbox::load(byte *src) {
	_base = src;

	type = *src++;
	orientation = *src++;
	rect.load(src);
	src += 8;
	for (byte i = 0; i < 4; ++i)
		sideHint[i] = *src++;
}

void Walkbox::save() const {
	assert(_base != NULL);
	_base[0] = type;
	_base[1] = orientation;
	rect.save();
}

void FadeType::load(byte *src) {
	rect.load(src);
	src += 8;
	value = *src;
}

} // End of namespace TeenAgent
