//--------------------------------------------------------------------------------------
// Math convenience functions
//--------------------------------------------------------------------------------------

#ifndef _MATH_HELPERS_H_DEFINED_
#define _MATH_HELPERS_H_DEFINED_

#include <cmath>
#include <stdint.h>


// Surprisingly, pi is not *officially* defined anywhere in C++
const float PI = 3.14159265359f;



// Test if a float value is approximately 0
// Epsilon value is the range around zero that is considered equal to zero
const float EPSILON = 0.5e-6f; // For 32-bit floats, requires zero to 6 decimal places
inline bool IsZero(const float x)
{
    return std::abs(x) < EPSILON;
}


// 1 / Sqrt. Used often (e.g. normalising) and can be optimised, so it gets its own function
inline float InvSqrt(const float x)
{
    return 1.0f / std::sqrt(x);
}


// Pass an angle in degrees, returns the angle in radians
constexpr float ToRadians(float d)
{
    return  d * PI / 180.0f;
}

// Pass an angle in radians, returns the angle in degrees
constexpr float ToDegrees(float r)
{
    return  r * 180.0f / PI;
}



// Return random integer from a to b (inclusive)
// Can only return up to RAND_MAX different values, spread evenly across the given range
// RAND_MAX is defined in stdlib.h and is compiler-specific (32767 on VS-2005, higher elsewhere)
inline uint32_t Random(const uint32_t a, const uint32_t b)
{
	// Could just use a + rand() % (b-a), but using a more complex form to allow range
	// to exceed RAND_MAX and still return values spread across the range
	uint32_t t = (b - a + 1) * rand();
	return t == 0 ? a : a + (t - 1) / RAND_MAX;
}

// Return random 32-bit float from a to b (inclusive)
// Can only return up to RAND_MAX different values, spread evenly across the given range
// RAND_MAX is defined in stdlib.h and is compiler-specific (32767 on VS-2005, higher elsewhere)
inline float Random(const float a, const float b)
{
	return a + (b - a) * (static_cast<float>(rand()) / RAND_MAX);
}

// Return random 64-bit float from a to b (inclusive)
// Can only return up to RAND_MAX different values, spread evenly across the given range
// RAND_MAX is defined in stdlib.h and is compiler-specific (32767 on VS-2005, higher elsewhere)
inline double Random(const double a, const double b)
{
	return a + (b - a) * (static_cast<double>(rand()) / RAND_MAX);
}

#endif // _MATH_HELPERS_H_DEFINED_
