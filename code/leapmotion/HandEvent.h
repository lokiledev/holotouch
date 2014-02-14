#ifndef HANDEVENT_H
#define HANDEVENT_H

#include <QEvent>
#include "leapmotion/Leap.h"

using namespace Leap;

//Custom events must be > QEvent::User (1000)
#define EVENT_OPEN 1002
#define EVENT_CLOSE 1003
#define EVENT_CLICK 1004
#define EVENT_DOUBLE_CLICK 1005

//defining a custom Event
class HandEvent: public QEvent
{
public:

    typedef enum {SINGLE, MULTIPLE} Selection_t;

    //create naming for our custom events
    static const QEvent::Type Opened = static_cast<QEvent::Type>(EVENT_OPEN);
    static const QEvent::Type Closed = static_cast<QEvent::Type>(EVENT_CLOSE);
    static const QEvent::Type Clicked = static_cast<QEvent::Type>(EVENT_CLICK);
    static const QEvent::Type DoubleClicked = static_cast<QEvent::Type>(EVENT_DOUBLE_CLICK);

private:
    Leap::Vector pos_; // palm position
    int itemSelected_;
    Selection_t selection_;

public:
    //types must be greater than User
    HandEvent(QEvent::Type pType = QEvent::User,
              Leap::Vector pPos = Leap::Vector(),
              int pSelect = -1,
              Selection_t pSelectMode = SINGLE);
    Leap::Vector pos();
    int item();
    HandEvent::Selection_t selectMode();
};




#endif // HANDEVENT_H
