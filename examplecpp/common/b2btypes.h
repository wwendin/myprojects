#pragma once
//
// File of common types used across many classes.  Only add values here
//		that are pretty common across the project.
//
#include <stdint.h>

namespace b2b {
	typedef float Distance;  	// meters (m for short)
	typedef float Magnitude;  	// unitless (it is up to user to define)
	typedef float Speed;  		// meters/sec (m/s for short)

	typedef float Angle;	 	// degrees (deg for short)
	typedef float AngularVelocity; // degrees/sec (deg/s or dps for short)
	typedef Angle Heading;	 	// degrees (deg for short)

	typedef float Temperature;	// degrees C
	typedef float Mass;			// kg

	typedef uint32_t Timestamp;	// milliseconds (ms for short)
    typedef std::vector<Timestamp> Timestamps;
	typedef uint32_t TimeMS;	// milliseconds (ms for short)
	typedef uint32_t TimeSec;	// seconds (s or sec for short)

	typedef uint32_t Color;		// 24-bit RGB (R,G,B are 8-bit), right justified
	const Color COLOR_INVALID = 0xFFFFFFFF;	// INVALID is impossible value
	  #define RGBToColor(r, g, b)		(((r) << 16) | ((g) << 8) | (b))
	  inline void ColorToRGB(Color color,
							uint8_t *pRed, uint8_t *pGreen, uint8_t *pBlue)
	  {
		*pRed = (color >> 16) & 0xff;
		*pGreen = (color >> 8) & 0xff;
		*pBlue = color & 0xff;
	  }

	// Volume is an integer range.  It's up to the user to define (it could
	// be 0 to 10, 0 to 100, etc) in each case where this type is used.
	typedef int16_t Volume;

};
