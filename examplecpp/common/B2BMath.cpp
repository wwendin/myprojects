//
// See B2BMath.h for documentation
//
#include <vector>

#include "B2BMath.h"

long B2BMath::RoundToInt(double t)
{
	// looking at ARM assembly, more efficient than calling floor/ceil
	return static_cast<long>B2BMATH_ROUND_TO_INT(t);
}
long B2BMath::RoundToInt(float t)
{
	// looking at ARM assembly, more efficient than calling double version
	return static_cast<long>B2BMATH_ROUND_FLOAT_TO_INT(t);
}

long B2BMath::DivRoundClosestInt(long dividend, long divisor)
{
	return DIV_ROUND_CLOSEST(dividend, divisor);
}

b2b::Heading B2BMath::InvertHeading(b2b::Heading heading)
{
	return (heading >= 180) ? (heading-180) : (heading+180);
}

b2b::Heading B2BMath::ResolveHeading(b2b::Heading heading)
{
	if (heading < 0) {
		while (heading < 0)
			heading += 360;
	} if (heading >= 360) {
		while (heading >= 360)
			heading -= 360;
	}

	return heading;
}

double B2BMath::DegreesToRadians(double degrees)
{

	return degrees * M_PI / 180.0;
}
double B2BMath::RadiansToDegrees(double radians)
{
	return radians * 180.0 / M_PI;
}

double B2BMath::Magnitude(double U, double V, double W)
{
	return sqrt((U*U)+(V*V)+(W*W));
}

double B2BMath::Magnitude(double U, double V)
{
	return sqrt((U*U)+(V*V));
}

double B2BMath::Dot(VectorRect v0, VectorRect v1)
{
	return (v0.U*v1.U) + (v0.V*v1.V);
}

double B2BMath::Dot(Vector3DRect v0, Vector3DRect v1)
{
	return (v0.U*v1.U) + (v0.V*v1.V) + (v0.W*v1.W);
}

double B2BMath::Magnitude(VectorRect v)
{
	return sqrt((v.U*v.U)+(v.V*v.V));
}

double B2BMath::Magnitude(Vector3DRect v)
{
	return sqrt((v.U*v.U)+(v.V*v.V)+(v.W*v.W));
}

double B2BMath::DistanceFromPointToLine(Vector3DRect p, 
											Vector3DRect v0, Vector3DRect v1,
											Vector3DRect *pClosestPoint)
{
	//	|(v1-v0)x(p-v0)| / |v1-v0|
	//			where x is cross product and || means magnitude
	//
	Vector3DRect v = v1 - v0;
    Vector3DRect w = p - v0;

    double c1 = B2BMath::Dot(w, v);
    double c2 = B2BMath::Dot(v, v);
    double b = c1 / c2;

	// Calculate point on line, pb, that is closest to p
    Vector3DRect pb = v0 + (v * b);
	if (pClosestPoint) *pClosestPoint = pb;

    return B2BMath::Magnitude(p - pb);
}

double B2BMath::DistanceFromPointToLine(VectorRect p, 
											VectorRect v0, VectorRect v1)
{
	// If the line passes through two points v0 and v1 then
	// the distance of p from the line is:
	// (v0.y-v1.y)*p.x + (v1.x-v0.x)*p.y + v0.x*v1.y - v1.x*v0.y 
	// ---------------------------------------
	//     distance from v0 to v1
	// Sign of numerator can tell you if you are right or left of line, 
	// looking from v0 to v1.   We don't care about that here
    double denominator = B2BMath::Magnitude(v1 - v0);

	// Get coefficients of the implicit line equation.
    double a = v0.U - v1.U;
    double b = v1.V - v0.V;
    double c = (v0.V * v1.U) - (v1.V * v0.U);

	// Calc numerator from above equation (See comments above)
    double numerator = (a * p.V) + (b * p.U) + c;

	return fabs(numerator / denominator); // we don't do signed distance
}

B2BMath::Vector3DRect::operator Vector3D() const
{
	Vector3D temp;
	temp.magnitude = Magnitude();
	temp.angle = RadiansToDegrees(atan2(V, U));
	if (temp.magnitude != 0)
	{
		temp.azimuth = RadiansToDegrees(asin(-W / temp.magnitude));
	}
	else
		temp.azimuth = 0; // choose something
	return temp;
}

// FIXME: improve speed by cache'ing trig values
B2BMath::Vector3D B2BMath::Vector3D::operator+(const Vector3DRect &rect) const
{
	// REMEMBER!!! W is positive down, negative up

	// Create a Vector3DRect from this (spherical coordinates)
	Vector3DRect thisRect = *this;

	thisRect += rect;

	// convert cartesian back to spherical
	Vector3D temp = thisRect;
	return temp;
}

B2BMath::Vector3D::operator Vector3DRect() const
{
	Vector3DRect thisRect;
	thisRect.W = -(magnitude * sin(DegreesToRadians(azimuth)));
	double horizHypotenuse = magnitude * cos(DegreesToRadians(azimuth));
	thisRect.U = horizHypotenuse * cos(DegreesToRadians(angle));
	thisRect.V = horizHypotenuse * sin(DegreesToRadians(angle));

	return thisRect;
}

const char *B2BMath::PlatformAxisToString(PlatformAxis val)
{
	switch (val) {
	  case PLATAXIS_U:
	  	return "U";
	  case PLATAXIS_V:
	  	return "V";
	  case PLATAXIS_W:
	  	return "W";
	  default:
	  	return "UNK";
	}
}

B2BMath::PlatformAxis B2BMath::CharToPlatformAxis(char c)
{
	switch (c) {
	  case 'U':
	  	return PLATAXIS_U;
	  case 'V':
	  	return PLATAXIS_V;
	  case 'W':
	  	return PLATAXIS_W;
	  default:
	  	return PLATAXIS_TOTAL;
	}
}

template<typename T, typename Iter_T>
bool B2BMath::Stats::CalcStats(Iter_T first, Iter_T last, double *pSum, 
					T *pMin, T *pMax,
					T *pMean, T *pStdDev, T *pVariance,
					bool sample)
{
	uint32_t n = distance(first, last);
	if (n == 0) 
		return false; // FAIL: no data

	double sum = 0;
	double sumSquares = 0;
	Iter_T iter;
	for (iter = first; iter != last; ++iter) {
		if (iter == first) {
			*pMax = *iter;
			*pMin = *iter;
		} else {
			if (*iter > *pMax) *pMax = *iter;
			if (*iter < *pMin) *pMin = *iter;
		}
		sum += *iter;
		sumSquares += (*iter) * (*iter);
	}
	*pSum = sum;
	*pMean = *pSum / n;


	// IMPROVE: this may not work for cases where T is floating point
	// 			and sumSquares and (sum^2)/n are very similar numbers.
    *pVariance = (sumSquares - ((sum * sum)/n)) / (sample ? (n-1) : n);
	*pStdDev = sqrt(*pVariance);

	return false; // FAIL: no data
}

template bool B2BMath::Stats::CalcStats<float, std::deque<float>::iterator>
			(std::deque<float>::iterator first,
			std::deque<float>::iterator last,
			double *pSum, float *pMin, float *pMax,
			float *pMean, float *pStdDev, float *pVariance,
			bool sample);
template bool B2BMath::Stats::CalcStats<b2b::TimeMS, std::deque<b2b::TimeMS>::iterator>
			(std::deque<b2b::TimeMS>::iterator first,
			std::deque<b2b::TimeMS>::iterator last,
			double *pSum, b2b::TimeMS *pMin, b2b::TimeMS *pMax,
			b2b::TimeMS *pMean, b2b::TimeMS *pStdDev, b2b::TimeMS *pVariance,
			bool sample);
