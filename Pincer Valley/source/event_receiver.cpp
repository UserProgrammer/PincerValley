#include "event_receiver.h"

EventReceiver::EventReceiver(SAppContext &context):
Context(context), cursorMoving(false), isLMouseDown(false), isRMouseDown(false), isLMousePressed(-1), isRMousePressed(-1)
{
    for(u32 n=0; n<KEY_KEY_CODES_COUNT; ++n){isKeyDown[n]=false;}
    for(u32 n=0; n<KEY_KEY_CODES_COUNT; ++n){isKeyPressed[n]=-1;}
}

EventReceiver::~EventReceiver(){}

bool EventReceiver::OnEvent(const SEvent& event)
{
    // Remember whether each key is down or up
    if(event.EventType==EET_KEY_INPUT_EVENT)
    {
        isKeyDown[event.KeyInput.Key]=event.KeyInput.PressedDown;
        isKeyPressed[event.KeyInput.Key]=event.KeyInput.PressedDown;
    }
    if(event.EventType==EET_MOUSE_INPUT_EVENT)
    {
        switch(event.MouseInput.Event)
        {
            case EMIE_MOUSE_MOVED:
            {
                cursorMoving=true;
            }break;

            case irr::EMIE_LMOUSE_PRESSED_DOWN:
            {
                isLMouseDown=true;
                isLMousePressed=true;
            }break;

            case irr::EMIE_LMOUSE_LEFT_UP:
            {
                isLMouseDown=false;
                isLMousePressed=false;
            }break;

            case irr::EMIE_RMOUSE_PRESSED_DOWN:
            {
                isRMouseDown=true;
                isRMousePressed=true;
            }break;

            case irr::EMIE_RMOUSE_LEFT_UP:
            {
                isRMouseDown=false;
                isRMousePressed=false;
            }break;

            default:
            break;
        }
    }
    return false;
}

bool EventReceiver::IsKeyDown(EKEY_CODE keyCode){return isKeyDown[keyCode];}

int EventReceiver::IsKeyPressed(EKEY_CODE keyCode){return isKeyPressed[keyCode];}

void EventReceiver::resetKeyPressed(){for(u32 n=0; n<KEY_KEY_CODES_COUNT; ++n)isKeyPressed[n]=-1;}

bool EventReceiver::IsMouseButtonDown(Mouse_Button button)
{
    if(button==M_LEFT){return isLMouseDown;}
    else{return isRMouseDown;}
}

int EventReceiver::IsMouseButtonPressed(Mouse_Button button)
{
    if(button==M_LEFT){return isLMousePressed;}
    else{return isRMousePressed;}
}

void EventReceiver::resetMousePressed()
{
    isLMousePressed=-1;
    isRMousePressed=-1;
}

bool EventReceiver::isCursorMoving(){return cursorMoving;}

void EventReceiver::resetCursorMove(){cursorMoving=false;}
