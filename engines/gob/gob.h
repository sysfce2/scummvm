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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_GOB_H
#define GOB_GOB_H

#include "common/random.h"
#include "common/system.h"

#include "graphics/pixelformat.h"

#include "engines/engine.h"

#include "gob/console.h"
#include "gob/detection/detection.h"

/**
 * This is the namespace of the Gob engine.
 *
 * Status of this engine: Supported
 *
 * Games using this engine:
 * - Adi 1
 * - Adi 2
 * - Adi 4
 * - Adi 5
 * - Adibou 1
 * - Adibou 2
 * - Adibou 3
 * - Adibou présente Cuisine
 * - Adibou présente Dessin
 * - Adibou présente Magie
 * - Adiboud'chou a la mer
 * - Adiboud'chou sur la banquise
 * - Adiboud'chou a la campagne
 * - Adiboud'chou dans la jungle et la savane
 * - English Fever
 * - Gobliiins
 * - Gobliins 2
 * - Goblins 3
 * - Ween: The Prophecy
 * - Bargon Attack
 * - Le pays des Pierres Magiques
 * - Lost in Time
 * - Nathan Vacances CM1/CE2
 * - The Bizarre Adventures of Woodruff and the Schnibble
 * - Fascination
 * - Inca II: Nations of Immortality
 * - Urban Runner
 * - Bambou le sauveur de la jungle
 * - Playtoons 1 Uncle Archibald
 * - Playtoons 2 The Case of the Counterfeit Collaborator (Spirou)
 * - Playtoons 3 The Secret of the Castle
 * - Playtoons 4 The Mandarin Prince
 * - Playtoons 5 The Stone of Wakan
 * - Playtoons Construction Kit 1 Monsters
 * - Playtoons Construction Kit 2 Knights
 * - Playtoons Construction Kit 3 The Far West
 * - Geisha
 * - Once Upon A Time: Abracadabra
 * - Once Upon A Time: Baba Yaga
 * - Once Upon A Time: Little Red Riding Hood
 * - Croustibat
 */

class GobMetaEngine;

namespace Gob {

class Game;
class Sound;
class Video;
class Global;
class Draw;
class DataIO;
class Goblin;
class VideoPlayer;
class Init;
class Inter;
class Map;
class Mult;
class PalAnim;
class Scenery;
class Util;
class SaveLoad;
class PreGob;

#define WRITE_VAR_UINT32(var, val)  _vm->_inter->_variables->writeVar32(var, val)
#define WRITE_VAR_UINT16(var, val)  _vm->_inter->_variables->writeVar16(var, val)
#define WRITE_VAR_UINT8(var, val)   _vm->_inter->_variables->writeVar8(var, val)
#define WRITE_VAR_STR(var, str)     _vm->_inter->_variables->writeVarString(var, str)
#define WRITE_VARO_UINT32(off, val) _vm->_inter->_variables->writeOff32(off, val)
#define WRITE_VARO_UINT16(off, val) _vm->_inter->_variables->writeOff16(off, val)
#define WRITE_VARO_UINT8(off, val)  _vm->_inter->_variables->writeOff8(off, val)
#define WRITE_VARO_STR(off, str)    _vm->_inter->_variables->writeOffString(off, str)
#define READ_VAR_UINT32(var)        _vm->_inter->_variables->readVar32(var)
#define READ_VAR_UINT16(var)        _vm->_inter->_variables->readVar16(var)
#define READ_VAR_UINT8(var)         _vm->_inter->_variables->readVar8(var)
#define READ_VARO_UINT32(off)       _vm->_inter->_variables->readOff32(off)
#define READ_VARO_UINT16(off)       _vm->_inter->_variables->readOff16(off)
#define READ_VARO_UINT8(off)        _vm->_inter->_variables->readOff8(off)
#define GET_VAR_STR(var)            _vm->_inter->_variables->getAddressVarString(var)
#define GET_VARO_STR(off)           _vm->_inter->_variables->getAddressOffString(off)
#define GET_VAR_FSTR(var)           _vm->_inter->_variables->getAddressVarString(var)
#define GET_VARO_FSTR(off)          _vm->_inter->_variables->getAddressOffString(off)

#define WRITE_VAR_OFFSET(off, val)  WRITE_VARO_UINT32((off), (val))
#define WRITE_VAR(var, val)         WRITE_VAR_UINT32((var), (val))
#define VAR_OFFSET(off)             READ_VARO_UINT32(off)
#define VAR(var)                    READ_VAR_UINT32(var)


// WARNING: Reordering these will invalidate save games!
enum Endianness {
	kEndiannessLE,
	kEndiannessBE
};

enum EndiannessMethod {
	kEndiannessMethodLE,     ///< Always little endian.
	kEndiannessMethodBE,     ///< Always big endian.
	kEndiannessMethodSystem, ///< Follows system endianness.
	kEndiannessMethodAltFile ///< Different endianness in alternate file.
};

enum {
	kDebugFuncOp = 1,
	kDebugDrawOp,
	kDebugGobOp,
	kDebugSound,
	kDebugExpression,
	kDebugGameFlow,
	kDebugFileIO,
	kDebugSaveLoad,
	kDebugGraphics,
	kDebugVideo,
	kDebugHotspots,
	kDebugDemo,
};

class GobEngine : public Engine {
private:
	GameType _gameType;
	int32 _features;
	Common::Platform _platform;
	const char *_extra;

	EndiannessMethod _endiannessMethod;

	uint32 _pauseStart;

	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	void pauseEngineIntern(bool pause) override;
	void syncSoundSettings() override;

	Common::Error initGameParts();
	Common::Error initGraphics();

	void deinitGameParts();

public:
	static const Common::Language _gobToScummVMLang[];

	Common::RandomSource _rnd;

	Common::Language _language;
	uint16 _width;
	uint16 _height;
	uint8 _mode;

	Graphics::PixelFormat _pixelFormat;

	Common::String _startStk;
	Common::String _startTot;
	uint32 _demoIndex;

	bool _copyProtection;
	bool _noMusic;

	GobConsole *_console;

	bool _resourceSizeWorkaround;
	bool _enableAdibou2FreeBananasWorkaround;
	bool _enableAdibou2FlowersInfiniteLoopWorkaround;

	Global *_global;
	Util *_util;
	DataIO *_dataIO;
	Game *_game;
	Sound *_sound;
	Video *_video;
	Draw *_draw;
	Goblin *_goblin;
	Init *_init;
	Map *_map;
	Mult *_mult;
	PalAnim *_palAnim;
	Scenery *_scenery;
	Inter *_inter;
	SaveLoad *_saveLoad;
	VideoPlayer *_vidPlayer;
	PreGob *_preGob;

	const char *getLangDesc(int16 language) const;
	void validateLanguage();
	void validateVideoMode(int16 videoMode);

	void pauseGame();

	EndiannessMethod getEndiannessMethod() const;
	Endianness getEndianness() const;
	Common::Platform getPlatform() const;
	GameType getGameType() const;
	bool isCD() const;
	bool isEGA() const;
	bool hasAdLib() const;
	bool isSCNDemo() const;
	bool isBATDemo() const;
	bool is640x400() const;
	bool is640x480() const;
	bool is800x600() const;
	bool is16Colors() const;
	bool isTrueColor() const;
	bool isDemo() const;

	bool hasResourceSizeWorkaround() const;

	bool isCurrentTot(const Common::String &tot) const;
	void setTrueColor(bool trueColor, bool convertAllSurfaces, Graphics::PixelFormat *format = nullptr);

	const Graphics::PixelFormat &getPixelFormat() const;

	GobEngine(OSystem *syst);
	~GobEngine() override;

	void initGame(const GOBGameDescription *gd);

	GameType getGameType(const char *gameId) const;
	bool gameTypeHasAddOns() const override;
	bool dirCanBeGameAddOn(const Common::FSDirectory &dir) const override;
	bool dirMustBeGameAddOn(const Common::FSDirectory &dir) const override;

	/**
	 * Used to obtain the game version as a fallback
	 * from our detection tables, if the VERSION file
	 * is missing
	 */
	const char *getGameVersion() const;
};

} // End of namespace Gob

#endif // GOB_GOB_H
