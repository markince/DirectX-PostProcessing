//--------------------------------------------------------------------------------------
// Vector4 class (cut down version), to hold points and vectors for matrix work
//--------------------------------------------------------------------------------------
// Code in .cpp file

#ifndef _CVECTOR4_H_DEFINED_
#define _CVECTOR4_H_DEFINED_

#include "CVector3.h"
#include "MathHelpers.h"
#include <cmath>

class CVector4
{
// Concrete class - public access
public:
    // Vector components
    float x;
	float y;
	float z;
	float w;

    /*-----------------------------------------------------------------------------------------
        Constructors
    -----------------------------------------------------------------------------------------*/

	// Default constructor - leaves values uninitialised (for performance)
	CVector4() {}

	// Construct with 4 values
	CVector4(const float xIn, const float yIn, const float zIn, const float wIn)
	{
		x = xIn;
		y = yIn;
		z = zIn;
		w = wIn;
	}
	
	// Construct with CVector3 and a float for the w value (use to initialise with points (w=1) and vectors (w=0))
	CVector4(const CVector3& vIn, const float wIn)
	{
		x = vIn.x;
		y = vIn.y;
		z = vIn.z;
		w = wIn;
	}
	
    // Construct using a pointer to 4 floats
    CVector4(const float* elts)
    {
        x = elts[0];
        y = elts[1];
        z = elts[2];
        w = elts[3];
    }


};


#endif // _CVECTOR3_H_DEFINED_
