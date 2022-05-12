//--------------------------------------------------------------------------------------
// ColourRGBA class (cut down version), to hold colours with float values
//--------------------------------------------------------------------------------------

#ifndef _COLOURRGBA_H_DEFINED_
#define _COLOURRGBA_H_DEFINED_

class ColourRGBA
{
// Concrete class - public access
public:
    // Vector components
    float r;
	float g;
	float b;
    float a;

	//--------------------------------------------------------------------------------------------

	// Default constructor - leaves values uninitialised (for performance)
    ColourRGBA() {}

	// Construct by value
    ColourRGBA( const float rIn, const float gIn, const float bIn, const float aIn = 1.0f)
	{
		r = rIn;
		g = gIn;
		b = bIn;
        a = aIn;
    }
	
	// Set the vector through a pointer to three floats
    void Set( const float* pfElts )
	{
		r = pfElts[0];
		g = pfElts[1];
		b = pfElts[2];
        a = pfElts[3];
    }
};
	
#endif // _COLOURRGBA_H_DEFINED_
