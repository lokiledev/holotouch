#ifndef LEAPLISTENER_H
#define LEAPLISTENER_H

#include <QObject>

#include "leapmotion/Leap.h"

using namespace Leap;

class LeapListener: public QObject, public Leap::Listener
{
    Q_OBJECT

public:
    //state machine for clic like gestures
    typedef enum {OPEN, CLOSE} handOpening_t;

     typedef enum {SINGLE, MULTIPLE} selectMode_t;

private:
    // remember the id of item in leap frames
    int32_t rightHand_;
    int32_t leftHand_;

    // id of item near the hand
    int trackedItem_;
    bool trackPrevious_;
    handOpening_t handState_;

    //right hand palm position
    Leap::Vector rPos_;

    selectMode_t selectionMode_;
    QObject* receiver_;

public:
    //ctor
    LeapListener();
    void setReceiver(QObject* pObject);

    //leap listener functions
    void onInit(const Controller&);
    void onConnect(const Controller&);
    void onDisconnect(const Controller&);
    void onExit(const Controller&);
    void onFrame(const Controller&);

    /*update the tracked item
     *called from the collision
     *detector
     */
    void setItem(int pNewItem);
};

#endif // LEAPLISTENER_H
