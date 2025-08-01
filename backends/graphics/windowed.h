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

#ifndef BACKENDS_GRAPHICS_WINDOWED_H
#define BACKENDS_GRAPHICS_WINDOWED_H

#include "backends/graphics/graphics.h"
#include "common/frac.h"
#include "common/rect.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "graphics/scaler/aspect.h"

enum {
	STRETCH_CENTER = 0,
	STRETCH_INTEGRAL = 1,
	STRETCH_INTEGRAL_AR = 2,
	STRETCH_FIT = 3,
	STRETCH_STRETCH = 4,
	STRETCH_FIT_FORCE_ASPECT = 5
};

enum {
	SCREEN_ALIGN_CENTER = 0,
	SCREEN_ALIGN_LEFT = 1,
	SCREEN_ALIGN_RIGHT = 2,
	SCREEN_ALIGN_XMASK = 3,
	SCREEN_ALIGN_MIDDLE = 0,
	SCREEN_ALIGN_TOP = 4,
	SCREEN_ALIGN_BOTTOM = 8,
	SCREEN_ALIGN_YMASK = 12
};

class WindowedGraphicsManager : virtual public GraphicsManager {
public:
	WindowedGraphicsManager() :
		_windowWidth(0),
		_windowHeight(0),
		_screenAlign(SCREEN_ALIGN_CENTER | SCREEN_ALIGN_MIDDLE),
		_rotationMode(Common::kRotationNormal),
		_overlayVisible(false),
		_overlayInGUI(false),
		_gameScreenShakeXOffset(0),
		_gameScreenShakeYOffset(0),
		_forceRedraw(false),
		_cursorVisible(false),
		_cursorX(0),
		_cursorY(0),
		_cursorNeedsRedraw(false),
		_cursorLastInActiveArea(true) {}

	void showOverlay(bool inGUI) override {
		_overlayInGUI = inGUI;

		if (inGUI) {
			_activeArea.drawRect = _overlayDrawRect;
			_activeArea.width = getOverlayWidth();
			_activeArea.height = getOverlayHeight();
		} else {
			_activeArea.drawRect = _gameDrawRect;
			_activeArea.width = getWidth();
			_activeArea.height = getHeight();
		}

		if (_overlayVisible)
			return;

		_overlayVisible = true;
		_forceRedraw = true;
		notifyActiveAreaChanged();
	}

	void hideOverlay() override {
		if (!_overlayVisible)
			return;

		_overlayInGUI = false;

		_activeArea.drawRect = _gameDrawRect;
		_activeArea.width = getWidth();
		_activeArea.height = getHeight();
		_overlayVisible = false;
		_forceRedraw = true;
		notifyActiveAreaChanged();
	}

	bool isOverlayVisible() const override { return _overlayVisible; }

	Common::Rect getSafeOverlayArea(int16 *width, int16 *height) const override {
		Insets insets = getSafeAreaInsets();

		// Create the overlay rect cut of the insets
		// in the window coordinate space
		// Make sure to avoid a negative size (and an invalid rect)
		const int safeLeft = MAX(_overlayDrawRect.left, insets.left),
		          safeTop = MAX(_overlayDrawRect.top, insets.top);
		Common::Rect safeArea(safeLeft, safeTop,
				MAX(safeLeft, MIN((int)_overlayDrawRect.right, _windowWidth - insets.right)),
				MAX(safeTop, MIN((int)_overlayDrawRect.bottom, _windowHeight - insets.bottom)));

		// Convert this safe area in the overlay coordinate space
		const int targetWidth = getOverlayWidth(),
		          targetHeight = getOverlayHeight(),
		          sourceWidth = _overlayDrawRect.width(),
		          sourceHeight = _overlayDrawRect.height();

		if (width) *width = targetWidth;
		if (height) *height = targetHeight;

		int rotatedTargetWidth = targetWidth,
		    rotatedTargetHeight = targetHeight;
		if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
			SWAP(rotatedTargetWidth, rotatedTargetHeight);
		}

		// First make it relative to overlay origin and scale it
		safeArea.left = ((safeArea.left - _overlayDrawRect.left) * rotatedTargetWidth) / sourceWidth;
		safeArea.top = ((safeArea.top - _overlayDrawRect.top) * rotatedTargetHeight) / sourceHeight;
		safeArea.right = ((safeArea.right - _overlayDrawRect.left) * rotatedTargetWidth) / sourceWidth;
		safeArea.bottom = ((safeArea.bottom - _overlayDrawRect.top) * rotatedTargetHeight) / sourceHeight;

		// Now rotate it
		switch (_rotationMode) {
		default:
		case Common::kRotationNormal:
			// Nothing to do
			break;
		case Common::kRotation90: {
			int16 tmp = safeArea.left;
			safeArea.left = safeArea.top;
			safeArea.top = rotatedTargetWidth - safeArea.right;
			//safeArea.right = targetWidth - (rotatedTargetHeight - safeArea.bottom);
			safeArea.right = safeArea.bottom; // targetWidth == rotatedTargetHeight
			safeArea.bottom = targetHeight - tmp;
			break;
		}
		case Common::kRotation180: {
			int16 tmp;
			tmp = safeArea.left;
			safeArea.left = rotatedTargetWidth - safeArea.right;
			safeArea.right = rotatedTargetWidth - tmp;
			tmp = safeArea.top;
			safeArea.top = rotatedTargetHeight - safeArea.bottom;
			safeArea.bottom = rotatedTargetHeight - tmp;
			break;
		}
		case Common::kRotation270: {
			int16 tmp = safeArea.left;
			safeArea.left = rotatedTargetHeight - safeArea.bottom;
			//safeArea.bottom = targetHeight - (rotatedTargetWidth - safeArea.right);
			safeArea.bottom = safeArea.right; // targetHeight == rotatedTargetWidth
			safeArea.right = targetWidth - safeArea.top;
			safeArea.top = tmp;
			break;
		}
		}

		return safeArea;
	}

	void setShakePos(int shakeXOffset, int shakeYOffset) override {
		if (_gameScreenShakeXOffset != shakeXOffset || _gameScreenShakeYOffset != shakeYOffset) {
			_gameScreenShakeXOffset = shakeXOffset;
			_gameScreenShakeYOffset = shakeYOffset;
			recalculateDisplayAreas();
			_cursorNeedsRedraw = true;
		}
	}

	int getWindowWidth() const { return _windowWidth; }
	int getWindowHeight() const { return _windowHeight; }

protected:
	/**
	 * @returns whether or not the game screen must have aspect ratio correction
	 * applied for correct rendering.
	 */
	virtual bool gameNeedsAspectRatioCorrection() const = 0;

	/**
	 * Backend-specific implementation for updating internal surfaces that need
	 * to reflect the new window size.
	 */
	virtual void handleResizeImpl(const int width, const int height) = 0;

	/**
	 * Converts the given point from the active virtual screen's coordinate
	 * space to the window's coordinate space (i.e. game-to-window or
	 * overlay-to-window).
	 */
	Common::Point convertVirtualToWindow(const int x, const int y) const {
		const int targetX = _activeArea.drawRect.left;
		const int targetY = _activeArea.drawRect.top;
		const int targetWidth = _activeArea.drawRect.width();
		const int targetHeight = _activeArea.drawRect.height();
		const int sourceWidth = _activeArea.width;
		const int sourceHeight = _activeArea.height;

		if (sourceWidth == 0 || sourceHeight == 0) {
			error("convertVirtualToWindow called without a valid draw rect");
		}

		int windowX, windowY;
		switch (_rotationMode) {
		default:
		case Common::kRotationNormal:
			windowX = targetX + (x * targetWidth + sourceWidth / 2) / sourceWidth;
			windowY = targetY + (y * targetHeight + sourceHeight / 2) / sourceHeight;
			break;
		case Common::kRotation90:
			windowX = targetX + ((y - (sourceHeight - 1)) * targetWidth + sourceHeight / 2) / sourceHeight;
			windowY = targetY + (x * targetHeight + sourceWidth / 2) / sourceWidth;
			break;
		case Common::kRotation180:
			windowX = targetX + ((x - (sourceWidth - 1)) * targetWidth + sourceWidth / 2) / sourceWidth;
			windowY = targetY + ((y - (sourceHeight - 1)) * targetHeight + sourceHeight / 2) / sourceHeight;
			break;
		case Common::kRotation270:
			windowX = targetX + (y * targetWidth + sourceHeight / 2) / sourceHeight;
			windowY = targetY + ((x - (sourceWidth - 1)) * targetHeight + sourceWidth / 2) / sourceWidth;
			break;
		}

		return Common::Point(CLIP<int>(windowX, targetX, targetX + targetWidth - 1),
		                     CLIP<int>(windowY, targetY, targetY + targetHeight - 1));
	}

	/**
	 * Converts the given point from the window's coordinate space to the
	 * active virtual screen's coordinate space (i.e. window-to-game or
	 * window-to-overlay).
	 */
	Common::Point convertWindowToVirtual(int x, int y) const {
		const int sourceX = _activeArea.drawRect.left;
		const int sourceY = _activeArea.drawRect.top;
		const int sourceMaxX = _activeArea.drawRect.right - 1;
		const int sourceMaxY = _activeArea.drawRect.bottom - 1;
		const int sourceWidth = _activeArea.drawRect.width();
		const int sourceHeight = _activeArea.drawRect.height();
		const int targetWidth = _activeArea.width;
		const int targetHeight = _activeArea.height;

		if (sourceWidth == 0 || sourceHeight == 0) {
			error("convertWindowToVirtual called without a valid draw rect");
		}

		x = CLIP<int>(x, sourceX, sourceMaxX);
		y = CLIP<int>(y, sourceY, sourceMaxY);

		int virtualX, virtualY;
		switch (_rotationMode) {
		default:
		case Common::kRotationNormal:
			virtualX = ((x - sourceX) * targetWidth + sourceWidth / 2) / sourceWidth;
			virtualY = ((y - sourceY) * targetHeight + sourceHeight / 2) / sourceHeight;
			break;
		case Common::kRotation90:
			virtualY = targetHeight - 1 - ((x - sourceX) * targetHeight + sourceWidth / 2) / sourceWidth;
			virtualX = ((y - sourceY) * targetWidth + sourceHeight / 2) / sourceHeight;
			break;
		case Common::kRotation180:
			virtualX = targetWidth - 1 - ((x - sourceX) * targetWidth + sourceWidth / 2) / sourceWidth;
			virtualY = targetHeight - 1 - ((y - sourceY) * targetHeight + sourceHeight / 2) / sourceHeight;
			break;
		case Common::kRotation270:
			virtualY = ((x - sourceX) * targetHeight + sourceWidth / 2) / sourceWidth;
			virtualX = targetWidth - 1 - ((y - sourceY) * targetWidth + sourceHeight / 2) / sourceHeight;
			break;
		}

		return Common::Point(CLIP<int>(virtualX, 0, targetWidth - 1),
		                     CLIP<int>(virtualY, 0, targetHeight - 1));
	}

	/**
	 * @returns the desired aspect ratio of the game surface.
	 */
	frac_t getDesiredGameAspectRatio() const {
		if (getHeight() == 0 || gameNeedsAspectRatioCorrection()) {
			return intToFrac(4) / 3;
		}

		return intToFrac(getWidth()) / getHeight();
	}

	/**
	 * @returns the scale used between the game size and the surface on which it is rendered.
	 */
	virtual int getGameRenderScale() const {
		return 1;
	}

	struct Insets {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	};

	/**
	 * Returns the insets needed to get a safe area which does not interfere
	 * with any system UI elements such as the notch or home indicator on mobile devices.
	 *
	 * @return The safe area insets
	 */
	virtual Insets getSafeAreaInsets() const {
		return {0, 0, 0, 0};
	}

	/**
	 * Called after the window has been updated with new dimensions.
	 *
	 * @param width The new width of the window, excluding window decoration.
	 * @param height The new height of the window, excluding window decoration.
	 */
	void handleResize(const int width, const int height) {
		_windowWidth = width;
		_windowHeight = height;
		handleResizeImpl(width, height);
	}

	/**
	 * Recalculates the display areas for the game and overlay surfaces within
	 * the window.
	 */
	virtual void recalculateDisplayAreas() {
		if (_windowHeight == 0) {
			return;
		}

		// Compute a safe area rectangle out of the insets
		Insets insets = getSafeAreaInsets();
		Common::Rect safeArea(insets.left, insets.top,
				_windowWidth - insets.right,
				_windowHeight - insets.bottom);

		// Create a game draw rect using the safe are dimensions
		populateDisplayAreaDrawRect(getDesiredGameAspectRatio(),
				getWidth() * getGameRenderScale(), getHeight() * getGameRenderScale(),
				safeArea, _gameDrawRect);

		// Move the game draw rect in the safe area
		_gameDrawRect.constrain(safeArea);

		if (getOverlayHeight()) {
			const int16 overlayWidth = getOverlayWidth(),
			            overlayHeight = getOverlayHeight();
			const frac_t overlayAspect = intToFrac(overlayWidth) / overlayHeight;
			populateDisplayAreaDrawRect(overlayAspect, overlayWidth, overlayHeight,
					Common::Rect(_windowWidth, _windowHeight),_overlayDrawRect);
		}

		if (_overlayInGUI) {
			_activeArea.drawRect = _overlayDrawRect;
			_activeArea.width = getOverlayWidth();
			_activeArea.height = getOverlayHeight();
		} else {
			_activeArea.drawRect = _gameDrawRect;
			_activeArea.width = getWidth();
			_activeArea.height = getHeight();
		}
		notifyActiveAreaChanged();
	}

	/**
	 * Sets the position of the hardware mouse cursor in the host system,
	 * relative to the window.
	 *
	 * @param x X coordinate in window coordinates.
	 * @param y Y coordinate in window coordinates.
	 */
	virtual void setSystemMousePosition(const int x, const int y) = 0;

	/**
	 * Called whenever the active area has changed.
	 */
	virtual void notifyActiveAreaChanged() {}

	bool showMouse(bool visible) override {
		if (_cursorVisible == visible) {
			return visible;
		}

		const bool last = _cursorVisible;
		_cursorVisible = visible;
		_cursorNeedsRedraw = true;
		return last;
	}

	/**
	 * Move ("warp") the mouse cursor to the specified position.
	 *
	 * @param x	The new X position of the mouse in virtual screen coordinates.
	 * @param y	The new Y position of the mouse in virtual screen coordinates.
	 */
	void warpMouse(int x, int y) override {
		// Check active coordinate instead of window coordinate to avoid warping
		// the mouse if it is still within the same virtual pixel
		const Common::Point virtualCursor = convertWindowToVirtual(_cursorX, _cursorY);
		if (virtualCursor.x != x || virtualCursor.y != y) {
			// Warping the mouse in SDL generates a mouse movement event, so
			// `setMousePosition` would be called eventually through the
			// `notifyMousePosition` callback if we *only* set the system mouse
			// position here. However, this can cause problems with some games.
			// For example, the cannon script in CoMI calls to warp the mouse
			// twice each time the cannon is reloaded, and unless we update the
			// mouse position immediately, the second call is ignored, which
			// causes the cannon to change its aim.
			const Common::Point windowCursor = convertVirtualToWindow(x, y);
			setMousePosition(windowCursor.x, windowCursor.y);
			setSystemMousePosition(windowCursor.x, windowCursor.y);
		}
	}

	/**
	 * Sets the position of the rendered mouse cursor in the window.
	 *
	 * @param x X coordinate in window coordinates.
	 * @param y Y coordinate in window coordinates.
	 */
	void setMousePosition(int x, int y) {
		if (_cursorX != x || _cursorY != y) {
			_cursorNeedsRedraw = true;
		}

		_cursorX = x;
		_cursorY = y;
	}

	/**
	 * The width of the window, excluding window decoration.
	 */
	int _windowWidth;

	/**
	 * The height of the window, excluding window decoration.
	 */
	int _windowHeight;

	/**
	 * How the overlay and game screens are aligned in the window.
	 * Centered vertically and horizontally by default.
	 */
	int _screenAlign;

	/**
	 * How the screens need to be rotated on the screen
	 */
	Common::RotationMode _rotationMode;

	/**
	 * Whether the overlay (i.e. launcher, including the out-of-game launcher)
	 * is visible or not.
	 */
	bool _overlayVisible;

	/**
	 * Whether when overlay is shown, mouse coordinates depend on window or game screen size
	 */
	bool _overlayInGUI;

	/**
	 * The offset by which the screen is moved horizontally.
	 */
	int _gameScreenShakeXOffset;

	/**
	* The offset by which the screen is moved vertically.
	*/
	int _gameScreenShakeYOffset;

	/**
	 * The scaled draw rectangle for the game surface within the window.
	 */
	Common::Rect _gameDrawRect;

	/**
	 * The scaled draw rectangle for the overlay (launcher) surface within the
	 * window.
	 */
	Common::Rect _overlayDrawRect;

	/**
	 * Data about the display area of a virtual screen.
	 */
	struct DisplayArea {
		/**
		 * The scaled area where the virtual screen is drawn within the window.
		 */
		Common::Rect drawRect;

		/**
		 * The width of the virtual screen's unscaled coordinate space.
		 */
		int width;

		/**
		 * The height of the virtual screen's unscaled coordinate space.
		 */
		int height;
	};

	/**
	 * Display area information about the currently active virtual screen. This
	 * will be the overlay screen when the overlay is active, and the game
	 * screen otherwise.
	 */
	DisplayArea _activeArea;

	/**
	 * Whether the screen must be redrawn on the next frame.
	 */
	bool _forceRedraw;

	/**
	 * Whether the cursor is actually visible.
	 */
	bool _cursorVisible;

	/**
	 * Whether the mouse cursor needs to be redrawn on the next frame.
	 */
	bool _cursorNeedsRedraw;

	/**
	 * Whether the last position of the system cursor was within the active area
	 * of the window.
	 */
	bool _cursorLastInActiveArea;

	/**
	 * The position of the mouse cursor, in window coordinates.
	 */
	int _cursorX, _cursorY;

private:
	void populateDisplayAreaDrawRect(const frac_t displayAspect, int originalWidth, int originalHeight, const Common::Rect &safeArea, Common::Rect &drawRect) const {
		int mode = getStretchMode();

		Common::Rect rotatedSafeArea(safeArea);
		int rotatedWindowWidth = _windowWidth,
		    rotatedWindowHeight = _windowHeight;

		if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
			SWAP(rotatedSafeArea.left, rotatedSafeArea.top);
			SWAP(rotatedSafeArea.right, rotatedSafeArea.bottom);
			SWAP(rotatedWindowWidth, rotatedWindowHeight);
		}
		const int rotatedSafeWidth = rotatedSafeArea.width(),
		    rotatedSafeHeight = rotatedSafeArea.height();

		// Mode Center   = use original size, or divide by an integral amount if window is smaller than game surface
		// Mode Integral = scale by an integral amount.
		// Mode Fit      = scale to fit the window while respecting the aspect ratio
		// Mode Stretch  = scale and stretch to fit the window without respecting the aspect ratio
		// Mode Fit Force Aspect = scale to fit the window while forcing a 4:3 aspect ratio
		int width = 0, height = 0;
		if (mode == STRETCH_CENTER || mode == STRETCH_INTEGRAL || mode == STRETCH_INTEGRAL_AR) {
			width = originalWidth;
			height = intToFrac(width) / displayAspect;
			if (width > rotatedSafeWidth || height > rotatedSafeHeight) {
				int fac = 1 + MAX((width - 1) / rotatedSafeWidth, (height - 1) / rotatedSafeHeight);
				width /= fac;
				height /= fac;
			} else if (mode == STRETCH_INTEGRAL) {
				int fac = MIN(rotatedSafeWidth / width, rotatedSafeHeight / height);
				width *= fac;
				height *= fac;
			}  else if (mode == STRETCH_INTEGRAL_AR) {
				int targetHeight = height;
				int horizontalFac = rotatedSafeWidth / width;
				do {
					width = originalWidth * horizontalFac;
					int verticalFac = (targetHeight * horizontalFac + originalHeight / 2) / originalHeight;
					height = originalHeight * verticalFac;
					--horizontalFac;
				} while (horizontalFac > 0 && height > rotatedSafeHeight);
				if (height > rotatedSafeHeight)
					height = targetHeight;
			}
		} else {
			frac_t windowAspect = intToFrac(rotatedSafeWidth) / rotatedSafeHeight;
			width = rotatedSafeWidth;
			height = rotatedSafeHeight;
			if (mode == STRETCH_FIT_FORCE_ASPECT) {
				frac_t ratio = intToFrac(4) / 3;
				if (windowAspect < ratio)
					height = intToFrac(width) / ratio;
				else if (windowAspect > ratio)
					width = fracToInt(height * ratio);
			} else if (mode != STRETCH_STRETCH) {
				if (windowAspect < displayAspect)
					height = intToFrac(width) / displayAspect;
				else if (windowAspect > displayAspect)
					width = fracToInt(height * displayAspect);
			}
		}

		int16 alignX, alignY;
		switch (_screenAlign & SCREEN_ALIGN_XMASK) {
			default:
			case SCREEN_ALIGN_CENTER:
				alignX = ((rotatedWindowWidth - width) / 2);
				break;
			case SCREEN_ALIGN_LEFT:
				alignX = 0;
				break;
			case SCREEN_ALIGN_RIGHT:
				alignX = (rotatedSafeArea.right - width);
				break;
		}

		switch (_screenAlign & SCREEN_ALIGN_YMASK) {
			default:
			case SCREEN_ALIGN_MIDDLE:
				alignY = ((rotatedWindowHeight - height) / 2);
				break;
			case SCREEN_ALIGN_TOP:
				alignY = 0;
				break;
			case SCREEN_ALIGN_BOTTOM:
				alignY = (rotatedSafeArea.bottom - height);
				break;
		}

		rotatedSafeArea.constrain(alignX, alignY, width, height);

		alignX += _gameScreenShakeXOffset * width / getWidth();
		alignY += _gameScreenShakeYOffset * height / getHeight();

		if (_rotationMode == Common::kRotation90 || _rotationMode == Common::kRotation270) {
			drawRect.top = alignX;
			drawRect.left = alignY;
			drawRect.setWidth(height);
			drawRect.setHeight(width);
		} else {
			drawRect.left = alignX;
			drawRect.top = alignY;
			drawRect.setWidth(width);
			drawRect.setHeight(height);
		}
	}
};

#endif
