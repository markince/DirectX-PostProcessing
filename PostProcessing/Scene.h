//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

//--------------------------------------------------------------------------------------
// Scene Geometry and Layout
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry();

// Layout the scene
// Returns true on success
bool InitScene();

// Release the geometry resources created above
void ReleaseResources();


//--------------------------------------------------------------------------------------
// Scene Render and Update
//--------------------------------------------------------------------------------------

void RenderScene(float frameTime);

// frameTime is the time passed since the last frame
void UpdateScene(float frameTime);


#endif //_SCENE_H_INCLUDED_
