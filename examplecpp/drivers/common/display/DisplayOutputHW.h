#pragma once
//
//  Display base class used by all classes that want to display something
//
#include <stdint.h>

#include "common/b2btypes.h"

#include "apps/common/B2BModule.h"
#include "apps/common/IOConfig.h"

class DisplayOutputHW : public B2BModule {
  public:
	//
	// name: name of this driver instance.  Must be unique.
	DisplayOutputHW(const char *name);
	virtual ~DisplayOutputHW();

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

	// If ready, then display HW is ready to use.  If not ready, calling
	// any method below (like DisplayImageFile) will have undefined results
	// including crashing
	void SetReady(bool ready) { m_isReady = ready; }
	bool IsReady() const { return m_isReady; }

	// Display an image from a file.  
	// This function displays the whole file without interruption.
	//
	// fileName: the full path name of the file containing .wav data
	//
	// RETURNS: true on success, false otherwise
	virtual bool DisplayImageFile(const char *fileName) = 0;

	// Fill entire display area with fillColor
	// RETURNS: true on success, false otherwise
	virtual bool FillScreen(b2b::Color fillColor) = 0;

	// Create a filled rectangle on display
	// col, row: column and row location of top left corner of rectangle
	// w: location of top left corner
	// fillColor: 24-bit color value
	// RETURNS: true on success, false otherwise
	virtual bool FillRect(uint16_t col, uint16_t row,
							uint16_t width, uint16_t height,
							b2b::Color fillColor) = 0;

	// Move write location to column, row
	virtual bool GoTo(uint16_t col, uint16_t row) = 0;

	// Draw one pixel to display HW or screen buffer
	// screenBuffer: if not NULL, write data to this screen buffer.  If NULL,
	//		then write data to display HW
	// col, row: column and row location of top left corner of rectangle
	// color: 24-bit color value
	// RETURNS: true on success, false otherwise
	virtual bool DrawPixel(uint16_t *screenBuffer, int16_t col, int16_t row,
							uint16_t color) = 0;
	// Same as above but color is defined by RGB
	virtual bool DrawPixel(uint16_t *screenBuffer, int16_t col, int16_t row,
							uint8_t r, uint8_t g, uint8_t b) = 0;

	typedef enum {
		ROTATION_NONE,
		ROTATION_90, // clockwise
		ROTATION_180,
		ROTATION_270, // clockwise
		ROTATION_TOTAL	// size of enum (never used as a valid value)
	} Rotation;
	// Set rotation for all draw routines (does not affect DisplayImageFile
	bool SetRotation(Rotation rotation)
	{
		if (rotation < ROTATION_TOTAL) {
			m_rotation = rotation;
			return true; // SUCCESS
		} else {
			return false; // FAIL
		}
	}
	Rotation GetRotation() { return m_rotation; }

  private:
	Rotation m_rotation;  // SetRotation/GetRotation
	bool m_isReady;
};
