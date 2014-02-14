#include "leapmotion/LeapListener.h"
#include "leapmotion/HandEvent.h"

#include <QApplication>
#include <QDebug>

// adapt the leapmotion coordinates to our application coordinates
#define SCALE_FACTOR_XY 30.0f
#define Z_OFFSET 100.0f
#define Z_SCALE_FACTOR 40.0f
#define Y_OFFSET 200.0f

#define SELECT_TRESHOLD 60.0f //hand opening in mm
#define RELEASE_TRESHOLD 80.0f //hand opening in mm

#define HOLD_TIME 10 //nb of frame with hand closed

LeapListener::LeapListener()
    : rightHand_(-1),
      leftHand_(-1),
      trackedItem_(-1),
      trackPrevious_(false),
      handState_(OPEN)
{
}

void LeapListener::onInit(const Controller& controller)
{
    Q_UNUSED(controller);
     qDebug() << "Initialized";
}

void LeapListener::onConnect(const Controller& controller)
{
    qDebug() << "Connected";
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void LeapListener::onDisconnect(const Controller& controller)
{
    //Note: not dispatched when running in a debugger.
    Q_UNUSED(controller);
    qDebug() << "Disconnected";
}

void LeapListener::onExit(const Controller& controller)
{
    Q_UNUSED(controller);
    qDebug() << "Exited";
}

void LeapListener::onFrame(const Controller& controller)
{
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();

    if (frame.hands().count() >= 1)
    {
        Hand hand = frame.hands()[0];
        Vector pos = hand.palmPosition();


        //closed hand hard to detect
        // closed = select cube
        float handOpening = 0;
        if ( hand.fingers().isEmpty() )
            handOpening = 0;
        else
            handOpening = hand.sphereRadius();

        //small state machine to detect closed/opened hand
        //use an hysteresis on hand sphere radius
        static int countClose = 0;
        static int countUp = 0;
        static bool clicked = false;
        /* If hand is not near the same item, reset counters
         */
        if ( !trackPrevious_ )
        {
            countClose = 0;
            countUp = 0;
            trackPrevious_ = true;
        }

        //State machine to detect click
        switch(handState_)
        {
        case OPEN:
            if (handOpening <= SELECT_TRESHOLD)
            {
                countUp++;
                if ( countUp >= HOLD_TIME )
                {
                    handState_ = CLOSE;
                    countUp = 0;
                }
            }
            else
                countUp = 0;
            break;
        case CLOSE:
            if ( handOpening >= RELEASE_TRESHOLD )
            {
                countClose++;
                if ( countClose >= HOLD_TIME )
                {
                    handState_ = OPEN;
                    countClose = 0;
                    clicked = true;
                }
            }
            else
                countClose = 0;
            break;
        default:
            break;
        }


        //adjust to our view coordinates
        rPos_.x = pos.x/SCALE_FACTOR_XY;
        rPos_.y = (pos.y - Y_OFFSET)/SCALE_FACTOR_XY;
        rPos_.z = (pos.z - Z_OFFSET)/Z_SCALE_FACTOR;

        selectionMode_ = HandEvent::SINGLE;
        if (frame.hands().count() == 2)
        {
            Hand leftHand = frame.hands().leftmost();
            float radius = leftHand.sphereRadius();
            if ( radius <= SELECT_TRESHOLD )
                selectionMode_ = HandEvent::MULTIPLE;
        }

        if ( receiver_ )
        {
            HandEvent* event = 0;
            if ( clicked )
            {
                event = new HandEvent(HandEvent::Clicked, rPos_, trackedItem_, selectionMode_);
                clicked = false;
            }
            else if ( handState_ == CLOSE )
                event = new HandEvent(HandEvent::Closed, rPos_, trackedItem_);
            else if ( handState_ == OPEN )
                event = new HandEvent(HandEvent::Opened, rPos_, trackedItem_);
            QApplication::sendEvent(receiver_, event);
        }
    }
}

void LeapListener::setReceiver(QObject* pObject)
{
    receiver_ = pObject;
}

void LeapListener::setItem(int pNewItem)
{
    if ( !(trackedItem_ == pNewItem) )
    {
        trackPrevious_ = false;
        trackedItem_ = pNewItem;
    }
}
