#ifndef _DIRECT3D_SETUP_H_INCLUDED_
#define _DIRECT3D_SETUP_H_INCLUDED_

//--------------------------------------------------------------------------------------
// Initialisation of Direct3D and main resources
//--------------------------------------------------------------------------------------

// Returns false on failure
bool InitDirect3D();

// Release the memory held by all objects created
void ShutdownDirect3D();


#endif //_DIRECT3D_SETUP_H_INCLUDED_
