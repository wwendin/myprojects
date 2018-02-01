#pragma once
//
// B2BMath.h: namespace for all sorts of B2B math features
//
#include <cmath>
#include <deque>
#include <stdio.h>

#include "common/b2btypes.h"

namespace B2BMath
{
	//// Round t to closest *integer* value
	// The "0.5 fraction problem": this function always rounds away from zero.
	//   Symmetric around 0 (good for graphics): -x.5 to -x-1; +x.5 to +x+1
	// Macros are for callers who want to force inline and for sharing below.
	// Macros are also good for when caller wants to type cast result itself.
	#define B2BMATH_ROUND_TO_INT(val) (((val) >= 0) ? \
											((val) + 0.5) : ((val) - 0.5))
	#define B2BMATH_ROUND_FLOAT_TO_INT(val) (((val) >= 0) ? \
											((val) + 0.5f) : ((val) - 0.5f))
	long RoundToInt(double t);
	long RoundToInt(float t);

	////// Returns dividend/divisor rounded to closest integer using ONLY
	// integer math.  NOTE: This macro is copied from Linux
	// The "0.5 fraction problem": this function always rounds away from zero.
	//   Symmetric around 0 (good for graphics): 5/2 = 3, -5/2 = -3
	//   Macro derived from this C code:
	//   long addForRounding = divisor / 2;
	//	 if ((dividend < 0) ^ (divisor < 0)) addForRounding = -addForRounding;
	//	 return (dividend + addForRounding) / divisor;
	//	 Works for unsigned, but negative unsigned inputs give undefined results
	#define DIV_ROUND_CLOSEST(dividend, divisor) 	\
					((((dividend) < 0) ^ ((divisor) < 0)) ? \
						(((dividend) - ((divisor)/2)) / (divisor)) : \
						(((dividend) + ((divisor)/2)) / (divisor)))
	long DivRoundClosestInt(long dividend, long divisor);

	// Inverts heading.   Returns value 180 degrees different than
	// heading parameter.   Returned value is always >= 0 and < 360
	b2b::Heading InvertHeading(b2b::Heading heading); 

	// Converts heading to value >= 0 and < 360
	b2b::Heading ResolveHeading(b2b::Heading heading); 

	double DegreesToRadians(double degrees);
	double RadiansToDegrees(double radians);

	// forward
	class VectorRect;
	class Vector3DRect;

	// Magnitude of vectors, which is also distance from origin to the point
	// defined by the vector
	double Magnitude(double U, double V, double W);
	double Magnitude(double U, double V);
	double Magnitude(VectorRect v);
	double Magnitude(Vector3DRect v);


	// 2D vector using rectangular (cartesian) coordinates (see PlatformAxis)
	class VectorRect {
	  public:
	  	VectorRect() : U(0), V(0) {}
	  	VectorRect(b2b::Magnitude u, b2b::Magnitude v) :
			U(u), V(v)
			{}
			
	  	virtual ~VectorRect() {}

		b2b::Magnitude U, V; // unitless, can be whatever user wants

		b2b::Magnitude Magnitude() const
		{ return B2BMath::Magnitude(*this); }

		VectorRect operator+(const VectorRect &rhs) const
		{
			VectorRect temp = *this;
			temp.U += rhs.U;
			temp.V += rhs.V;
			return temp;
		}

		VectorRect &operator+=(const VectorRect &rhs)
		{
			*this = *this + rhs; // use operator+
			return *this;
		}

		VectorRect operator-(const VectorRect &rhs) const
		{
			VectorRect temp = *this;
			temp.U -= rhs.U;
			temp.V -= rhs.V;
			return temp;
		}

		VectorRect &operator-=(const VectorRect &rhs)
		{
			*this = *this - rhs; // use operator-
			return *this;
		}

		VectorRect operator*(float scalar) const
		{
			VectorRect temp = *this;
			temp.U *= scalar;
			temp.V *= scalar;
			return temp;
		}

		VectorRect &operator*=(float scalar)
		{
			*this = *this * scalar; // use operator*
			return *this;
		}

		VectorRect operator/(float scalar) const
		{
			VectorRect temp = *this;
			temp.U /= scalar;
			temp.V /= scalar;
			return temp;
		}

		VectorRect &operator/=(float scalar)
		{
			*this = *this / scalar; // use operator/
			return *this;
		}
	};

	// 3D vector using rectangular (cartesian) coordinates (see PlatformAxis)
	class Vector3D;
	class Vector3DRect : public VectorRect {
	  public:
	  	Vector3DRect() : W(0) {}
	  	Vector3DRect(b2b::Magnitude u, b2b::Magnitude v, b2b::Magnitude w) :
			VectorRect(u,v),
			W(w)
			{}
	  	virtual ~Vector3DRect() {}

		b2b::Magnitude W; // unitless, can be whatever user wants

		b2b::Magnitude Magnitude() const
		{ return B2BMath::Magnitude(*this); }

		Vector3DRect operator+(const Vector3DRect &rhs) const
		{
			Vector3DRect temp = *this;
			temp.U += rhs.U;
			temp.V += rhs.V;
			temp.W += rhs.W;
			return temp;
		}

		Vector3DRect &operator+=(const Vector3DRect &rhs)
		{
			*this = *this + rhs; // use operator+
			return *this;
		}

		Vector3DRect operator-(const Vector3DRect &rhs) const
		{
			Vector3DRect temp = *this;
			temp.U -= rhs.U;
			temp.V -= rhs.V;
			temp.W -= rhs.W;
			return temp;
		}

		Vector3DRect &operator-=(const Vector3DRect &rhs)
		{
			*this = *this - rhs; // use operator-
			return *this;
		}

		Vector3DRect operator*(float scalar) const
		{
			Vector3DRect temp = *this;
			temp.U *= scalar;
			temp.V *= scalar;
			temp.W *= scalar;
			return temp;
		}

		Vector3DRect &operator*=(float scalar)
		{
			*this = *this * scalar; // use operator*
			return *this;
		}


		Vector3DRect operator/(float scalar) const
		{
			Vector3DRect temp = *this;
			temp.U /= scalar;
			temp.V /= scalar;
			temp.W /= scalar;
			return temp;
		}

		Vector3DRect &operator/=(float scalar)
		{
			*this = *this / scalar; // use operator/
			return *this;
		}

		// Cast Vector3DRect to Vector3D
		operator Vector3D() const;
	};

	// 2D Vector using polar coordinates (origin is defined in PlatformAxis)
	// angle and magnitude define vector on horizontal plane.
	//
	// angle picture (looking straight down on vehicle)
	//		0 degrees is directly forward of center, 90 degrees is right of
	//		center, 180 is backwards, 270 is left.
	//
	//            -360
	//              0
	//
	//            mouth
	//   -90    /       \   -270
	//   270   | vehicle |   90
	//          \       /
	//            anus
	//
	//            180
	//           -180
	class Vector {
	  public:
	  	Vector() : magnitude(0), angle(0) {}
	  	Vector(b2b::Magnitude m, b2b::Angle h) : 
			magnitude(m), angle(h)
			{}
	  	virtual ~Vector() {}

		b2b::Magnitude magnitude; // unitless, can be whatever user wants
		b2b::Angle angle;

		double I() {return magnitude*cos(DegreesToRadians((angle)));}
		double J() {return magnitude*sin(DegreesToRadians((angle)));}

		//rotate the angle 180 degrees.
		void Invert() {angle = InvertHeading(angle);}

		Vector operator+(Vector &src)
		{
			double i = I() + src.I();
			double j = J() + src.J();
			if(j==0)
			{
				printf("ERROR bad arg J\n");
				return *this;
			}
			magnitude=sqrt(pow(i,2)+pow(j,2));
			angle=RadiansToDegrees(atan2(i,j));
			return *this;
		}

		Vector &operator+=(Vector &rhs)
		{
			*this = *this + rhs; // use operator+
			return *this;
		}

		virtual void Reset()
		{
			magnitude=0;
			angle=0;
		}
	};

	// Vector3D using spherical coordinates (origin is defined in PlatformAxis)
	// 3D vector uses normal 2D Vector class plus adds azimuth (which we 
	// could also call elevation as it is angle above U-V plane)
	// azimuth is degrees above horizontal plane.  0 is on horizontal
	// plane, 90 is straight up, -90 straight down.   azimuth range is
	// restricted to -90 to +90 degrees so that we don't have duplicate 
	// definitions for points in 3D space.
	// Another way to describe it: 2D vector angle and magnitude define vector
	// on horizonal plane and azimuth defines up/down angle of that vector.
	//
	class Vector3D : public Vector {
	  public:
	  	Vector3D() : azimuth(0) {}
	  	Vector3D(b2b::Magnitude m, b2b::Angle h, b2b::Angle a) : 
			Vector(m, h), azimuth(a)
			{}

		b2b::Angle azimuth; 


		Vector3D operator+(const Vector3DRect &rect) const;

		Vector3D &operator+=(const Vector3DRect &rect)
		{
			*this = *this + rect; // use operator+
			return *this;
		}

		// Cast Vector3D to Vector3DRect
		operator Vector3DRect() const;

		virtual void Reset()
		{
			Vector::Reset();
			azimuth=0;
		}
	};

	// Returns a vector of the line between the endpoints of the two vectors.
	// The resulting vector points from v1 to v2.
	inline Vector3DRect VectorLine(const Vector3DRect &v1, 
								   const Vector3DRect &v2)
		{ return v2 - v1; }
	inline Vector3D VectorLine(const Vector3D &v1, const Vector3D &v2)
		{ return VectorLine((Vector3DRect)v1, (Vector3DRect)v2); }

	// Returns a vector perpendicular to the given vector
	// PAY ATTENTION: There are infinite number of vectors perpendicular
	//		in 3D space.   We arbitrarily choose one use 2D method.
	//		Notice that we don't change W
	inline Vector3DRect VectorPerpendicular(const Vector3DRect &v)
	{
		Vector3DRect newv = v;
		b2b::Magnitude saveU = newv.U;
		newv.U = -newv.V;
		newv.V = saveU;
		return newv;
	}
	inline Vector3D VectorPerpendicular(const Vector3D &v)
		{ return VectorPerpendicular((Vector3DRect)v); }

	// Rectangular (cartesian) axes on our vehicle platform (the creature):
	// U axis:
	//		Positive values point forward.  
	//		Rotation around U axis is roll.  Positive roll rotation value means
	//		roll to right (when looking forward on vehicle).
	// V axis:
	//		Positive values point right as you face forward.
	//		Rotation around V axis is pitch.  Positive pitch rotation value 
	//		means the front of the vehicle moves up and rear down.
	// W axis: 
	//		Positive values point down.  
	//		Rotation around W axis is yaw.  Positive yaw rotation value means
	//		front turns to the right when looking forward on vehicle (clockwise
	//		when looking down on vehicle).
	//
	// The W axis goes through the center of the vehicle and that center
	// is defined as the screw in center of the DF Robot Cherokey chassis.
	// U and V axes lie on ground.  In other words, W of 0 is on the ground.
	//
	// This is a right-handed orthogonal system.
	// And all rotation follows the right-hand rule (positive rotation is
	// clockwise if you look along axis from negative to positive end of axis).
	//
	// Our vehicle axis system has U/V axes plane on the ground.   Therefore,
	// all offsets along W axis from ground up will have a negative W.
	//
	typedef enum {
		PLATAXIS_U = 0,
		PLATAXIS_V,
		PLATAXIS_W,
		PLATAXIS_TOTAL		// size of enum (never used as a valid value)
	} PlatformAxis;

	const char *PlatformAxisToString(PlatformAxis val);

	PlatformAxis CharToPlatformAxis(char c);

	namespace Stats {

		// Calculate all the stats
		// sample: if true calculate sample variance, if false calculate
		//			population variance
		// RETURNS: true on success, false otherwise.  If false is returned
		//			 no results are set in pointers.
		template<typename T, typename Iter_T>
		bool CalcStats(Iter_T first, Iter_T last, double *pSum, 
						T *pMin, T *pMax,
						T *pMean, T *pStdDev, T *pVariance,
						bool sample=true);
	}

	template <typename T> class Statistics {
	  public:
		// size: maximum number of 
		Statistics(uint8_t size) : 
			m_size(size),
			m_dataChangesSinceLastCalcStats(true) // stats are stale
			{}
		~Statistics() {}

		void Add(T value) 
		{
			if (m_data.size() == 0) {
				// This Add is the first
				m_minAll = value;
				m_maxAll = value;
			}
			m_data.push_back(value); // add newest data
			m_dataChangesSinceLastCalcStats = true; // stats are stale
			if (m_data.size() > m_size)
				m_data.pop_front(); // remove oldest data
		}

		// All stats are of current data set (not including old data discarded)
		bool CalcStats(double *pSum, 
						T *pMin, T *pMax,
						T *pMean, T *pStdDev, T *pVariance,
						bool sample=true)
		{
			bool returnVal = true; // assume SUCCESS
			if (m_dataChangesSinceLastCalcStats) {
				returnVal = Stats::CalcStats(m_data.begin(), m_data.end(), 
						&m_sum, &m_min, &m_max, &m_mean, &m_stdDev, &m_variance,
						sample);
				m_dataChangesSinceLastCalcStats = false; // stats are up-to-date
			}
			
			*pSum = m_sum;
			*pMin = m_min;
			if (m_min < m_minAll)
				m_minAll = m_min;
			*pMax = m_max;
			if (m_max > m_maxAll)
				m_maxAll = m_max;
			*pMean = m_mean;
			*pStdDev = m_stdDev;
			*pVariance = m_variance;

			return returnVal;
		}

		// Get historical min/max (calculated over all data Add'ed to stats
		// including old data)
		void MinMaxGet(T *pMinAll, T *pMaxAll) const
		{
			*pMinAll = m_minAll;
			*pMaxAll = m_maxAll;
		}

	  private:
		uint8_t m_size; // from ctor

		// Calculated by CalcStats and cached here
		double m_sum;
		T m_min;
		T m_max;
		T m_mean;
		T m_stdDev;
		T m_variance;

		// stats kept over the history of all data Add'ed to stats
		T m_minAll;
		T m_maxAll;

		// true if m_data has changed since last CalcStats call
		bool m_dataChangesSinceLastCalcStats; 
	    std::deque<T> m_data; // Our data
	};

	// Calculate dot products.  2D and 3D versions
	double Dot(VectorRect v0, VectorRect v1);
	double Dot(Vector3DRect v0, Vector3DRect v1);

	// Return distance from point p to a line defined by two points: v1 and v2
	// pClosestPoint: point that is closest to p on line.  This is optional,
	//		caller can set to NULL if this value is not desired.
	// 2D and 3D versions are here
	double DistanceFromPointToLine(VectorRect p, 
										 VectorRect v0, VectorRect v1);
	double DistanceFromPointToLine(Vector3DRect p, 
											Vector3DRect v0, Vector3DRect v1,
											Vector3DRect *pClosestPoint=NULL);
}
