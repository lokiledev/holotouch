#ifndef HANDEVENT_H
#define HANDEVENT_H

#include <QEvent>
#include "leapmotion/Leap.h"

using namespace Leap;

#define EVENT_OPEN 1002
#define EVENT_CLOSE 1003
#define EVENT_CLICK 1004
#define EVENT_DOUBLE_CLICK 1005

//defining a custom Event
class HandEvent: public QEvent
{
public:

    // distinguish left or right hand
    typedef enum {RIGHT, LEFT, ANY} HandId_t;

    //create naming for our custom events
    static const QEvent::Type Opened = static_cast<QEvent::Type>(EVENT_OPEN);
    static const QEvent::Type Closed = static_cast<QEvent::Type>(EVENT_CLOSE);
    static const QEvent::Type Clicked = static_cast<QEvent::Type>(EVENT_CLICK);
    static const QEvent::Type DoubleClicked = static_cast<QEvent::Type>(EVENT_DOUBLE_CLICK);

private:
    Vector pos_; // palm position
    HandId_t hand_ ;
    int itemSelected_;

public:
    //types must be greater than User
    HandEvent(QEvent::Type pType = QEvent::User,
              Vector pPos = Vector(),
              HandId_t pId = ANY, int pSelect = -1);
    Vector pos();
    HandId_t hand();
    int item();
};




#endif // HANDEVENT_H
