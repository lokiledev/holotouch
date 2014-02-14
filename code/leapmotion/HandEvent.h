#ifndef HANDEVENT_H
#define HANDEVENT_H

#include <QEvent>
#include "leapmotion/Leap.h"

using namespace Leap;

#define EVENT_OPEN 1002
#define EVENT_CLOSE 1003

//defining a custom Event
class HandEvent: public QEvent
{
public:

    // distinguish left or right hand
    typedef enum {RIGHT, LEFT, ANY} HandId_t;
    static const QEvent::Type OpenEvent = static_cast<QEvent::Type>(EVENT_OPEN);
    static const QEvent::Type CloseEvent = static_cast<QEvent::Type>(EVENT_CLOSE);

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
