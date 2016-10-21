#ifndef INCLUDE_EVENT_CLASS
#define INCLUDE_EVENT_CLASS

#include <iostream>
#include <irrlicht.h>
#include "dummy.h"

using namespace std;
using namespace irr;

enum Mouse_Button{M_LEFT=KEY_LBUTTON, M_RIGHT=KEY_RBUTTON};

struct SAppContext
{
    irr::IrrlichtDevice *device;
};

class EventReceiver:public irr::IEventReceiver
{
    public:
    EventReceiver(SAppContext &context);
    ~EventReceiver();

    virtual bool OnEvent(const irr::SEvent &event);
    /// Return the state of a specified keyboard key.
    virtual bool IsKeyDown(EKEY_CODE keyCode);
    /// Return the action of a specified keyboard key.
    int IsKeyPressed(EKEY_CODE keyCode);
    /// Reset variable that holds keyboard key actions (must be called at the end of the event loop in main.cpp)
    void resetKeyPressed();
    /// Return the state of the left mouse button.
    bool IsMouseButtonDown(Mouse_Button button);
    /// Return the state of the right mouse button.
    int IsMouseButtonPressed(Mouse_Button button);

    void resetMousePressed();

    bool isCursorMoving();

    void resetCursorMove();

    private:
    SAppContext &Context;
    bool isKeyDown[KEY_KEY_CODES_COUNT];
    int isKeyPressed[KEY_KEY_CODES_COUNT];
    bool cursorMoving;
    bool isLMouseDown, isRMouseDown;
    int isLMousePressed, isRMousePressed;
};
#endif

