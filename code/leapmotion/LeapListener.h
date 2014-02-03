#ifndef LEAPLISTENER_H
#define LEAPLISTENER_H

#include "Leap.h"
using namespace Leap;

class LeapListener : public Listener
{
    //leap listener functions
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
};

#endif // LEAPLISTENER_H
