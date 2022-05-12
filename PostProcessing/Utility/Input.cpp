//--------------------------------------------------------------------------------------
// Key/mouse input functions
// Used in the same way as the TL-Engine
//--------------------------------------------------------------------------------------

#include "Input.h"


//////////////////////////////////
// Globals

// Current state of all keys (and mouse buttons)
KeyState gKeyStates[NumKeyCodes];

// Current position of mouse
int gMouseX, gMouseY;



//////////////////////////////////
// Initialisation

void InitInput()
{
    // Initialise input data
    for (int i = 0; i < NumKeyCodes; ++i)
    {
        gKeyStates[i] = NotPressed;
    }

    gMouseX = gMouseY = 0;
}


//////////////////////////////////
// Events

// Event called to indicate that a key has been pressed down
void KeyDownEvent(KeyCode Key)
{
    if (gKeyStates[Key] == NotPressed)
    {
        gKeyStates[Key] = Pressed;
    }
    else
    {
        gKeyStates[Key] = Held;
    }
}

// Event called to indicate that a key has been lifted up
void KeyUpEvent(KeyCode Key)
{
   gKeyStates[Key] = NotPressed;
}

// Event called to indicate that the mouse has been moved
void MouseMoveEvent(int X, int Y)
{
    gMouseX = X;
    gMouseY = Y;
}


//////////////////////////////////
// Input functions

// Returns true when a given key or button is first pressed down. Use
// for one-off actions or toggles. Example key codes: Key_A or
// Mouse_LButton, see input.h for a full list.
bool KeyHit(KeyCode eKeyCode)
{
    if (gKeyStates[eKeyCode] == Pressed)
    {
        gKeyStates[eKeyCode] = Held;
        return true;
    }
    return false;
}

// Returns true as long as a given key or button is held down. Use for
// continuous action or motion. Example key codes: Key_A or
// Mouse_LButton, see input.h for a full list.
bool KeyHeld(KeyCode eKeyCode)
{
    if (gKeyStates[eKeyCode] == NotPressed)
    {
        return false;
    }
    gKeyStates[eKeyCode] = Held;
    return true;
}

    
// Returns current X position of mouse
int GetMouseX()
{
    return gMouseX;
}

// Returns current Y position of mouse
int GetMouseY()
{
    return gMouseY;
}
