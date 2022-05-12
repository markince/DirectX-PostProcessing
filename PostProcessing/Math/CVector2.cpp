//--------------------------------------------------------------------------------------
// Vector2 class (cut down version), mainly used for texture coordinates (UVs)
// but can be used for 2D points as well
//--------------------------------------------------------------------------------------

#include "CVector2.h"


/*-----------------------------------------------------------------------------------------
    Operators
-----------------------------------------------------------------------------------------*/

// Addition of another vector to this one, e.g. Position += Velocity
CVector2& CVector2::operator+= (const CVector2& v)
{
    x += v.x;
    y += v.y;
    return *this;
}

// Subtraction of another vector from this one, e.g. Velocity -= Gravity
CVector2& CVector2::operator-= (const CVector2& v)
{
    x -= v.x;
    y -= v.y;
    return *this;
}

// Negate this vector (e.g. Velocity = -Velocity)
CVector2& CVector2::operator- ()
{
    x = -x;
    y = -y;
    return *this;
}

// Plus sign in front of vector - called unary positive and usually does nothing. Included for completeness (e.g. Velocity = +Velocity)
CVector2& CVector2::operator+ ()
{
    return *this;
}


// Vector-vector addition
CVector2 operator+ (const CVector2& v, const CVector2& w)
{
    return CVector2{ v.x + w.x, v.y + w.y };
}

// Vector-vector subtraction
CVector2 operator- (const CVector2& v, const CVector2& w)
{
    return CVector2{ v.x - w.x, v.y - w.y };
}


/*-----------------------------------------------------------------------------------------
    Non-member functions
-----------------------------------------------------------------------------------------*/

// Dot product of two given vectors (order not important) - non-member version
float Dot(const CVector2& v1, const CVector2& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

// Return unit length vector in the same direction as given one
CVector2 Normalise(const CVector2& v)
{
    float lengthSq = v.x*v.x + v.y*v.y;

    // Ensure vector is not zero length (use function from MathHelpersh.h to check if float is approximately 0)
    if (IsZero(lengthSq))
    {
        return CVector2{ 0.0f, 0.0f };
    }
    else
    {
        float invLength = InvSqrt(lengthSq);
        return CVector2{ v.x * invLength, v.y * invLength };
    }
}
