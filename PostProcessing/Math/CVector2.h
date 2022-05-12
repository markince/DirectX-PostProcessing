//--------------------------------------------------------------------------------------
// Vector2 class (cut down version), mainly used for texture coordinates (UVs)
// but can be used for 2D points as well
//--------------------------------------------------------------------------------------
// Code in .cpp file

#ifndef _CVECTOR2_H_DEFINED_
#define _CVECTOR2_H_DEFINED_

#include "MathHelpers.h"
#include <cmath>

class CVector2
{
// Concrete class - public access
public:
    // Vector components
    float x;
    float y;

    /*-----------------------------------------------------------------------------------------
        Constructors
    -----------------------------------------------------------------------------------------*/

    // Default constructor - leaves values uninitialised (for performance)
    CVector2() {}

    // Construct with 2 values
    CVector2(const float xIn, const float yIn)
    {
        x = xIn;
        y = yIn;
    }

    // Construct using a pointer to 2 floats
    CVector2(const float* pfElts)
    {
        x = pfElts[0];
        y = pfElts[1];
    }


    /*-----------------------------------------------------------------------------------------
    Member functions
    -----------------------------------------------------------------------------------------*/

    // Addition of another vector to this one, e.g. Position += Velocity
    CVector2& operator+= (const CVector2& v);

    // Subtraction of another vector from this one, e.g. Velocity -= Gravity
    CVector2& operator-= (const CVector2& v);

    // Negate this vector (e.g. Velocity = -Velocity)
    CVector2& operator- ();

    // Plus sign in front of vector - called unary positive and usually does nothing. Included for completeness (e.g. Velocity = +Velocity)
    CVector2& operator+ ();
};


/*-----------------------------------------------------------------------------------------
    Non-member operators
-----------------------------------------------------------------------------------------*/

// Vector-vector addition
CVector2 operator+ (const CVector2& v, const CVector2& w);

// Vector-vector subtraction
CVector2 operator- (const CVector2& v, const CVector2& w);


/*-----------------------------------------------------------------------------------------
Non-member functions
-----------------------------------------------------------------------------------------*/

// Dot product of two given vectors (order not important) - non-member version
float Dot(const CVector2& v1, const CVector2& v2);

// Return unit length vector in the same direction as given one
CVector2 Normalise(const CVector2& v);


#endif // _CVECTOR3_H_DEFINED_
