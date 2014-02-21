#include "leapmotion/LeapListener.h"
#include "leapmotion/HandEvent.h"

#include <QApplication>
#include <QDebug>

#define SELECT_TRESHOLD 60.0f //hand opening in mm
#define RELEASE_TRESHOLD 80.0f //hand opening in mm
#define HOLD_TIME 10 //nb of frame with hand closed

#define ANGLE_ZOOM_TRESHOLD 20.0f // pitch of left hand in degrees
#define ZOOM_FACTOR 0.01f // each frame in zoom moves by this.
#define RESWIPE_INTERVAL 200 //minimum time between 2 swipes in ms

LeapListener::LeapListener()
    : rightHand_(-1),
      leftHand_(-1),
      trackedItem_(-1),
      trackPrevious_(false),
      handState_(OPEN),
      zoomFactor_(0)
{
    swipeTimer_ = new QTimer();
    swipeTimer_->setInterval(RESWIPE_INTERVAL);
    swipeTimer_->setSingleShot(true);
}

void LeapListener::onInit(const Controller& controller)
{
     qDebug() << "Initialized";
     Config config = controller.config();
     config.setFloat("Gesture.Circle.MinRadius", 30);
     config.setFloat("Gesture.Circle.MinArc", 2*PI);
     config.save();
}

void LeapListener::onConnect(const Controller& controller)
{
    qDebug() << "Connected";
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
    //controller.enableGesture(Gesture::TYPE_CIRCLE);
}

void LeapListener::onDisconnect(const Controller& controller)
{
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
        Hand hand = frame.hands().rightmost();
        rightHand_ = hand.id();
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
        static bool zoom = false;
        //If hand is not near the same item, reset counters


        if ( !trackPrevious_ )
        {
            countClose = 0;
            countUp = 0;
            trackPrevious_ = true;
        }

        //State machine to detect click
        if (! grabbing_ )
        {
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
                    }
                }
                else
                    countClose = 0;
                break;
            default:
                break;
            }
        }
        else // consider hand always closed
        {
            if ( handOpening >= RELEASE_TRESHOLD )
            {
                countClose++;
                if ( countClose >= HOLD_TIME )
                {
                    handState_ = OPEN;
                    countClose = 0;

                }
            }
        }

        InteractionBox box = frame.interactionBox();
        if ( box.isValid() )
            rPos_ = box.normalizePoint(pos, false);

        selectionMode_ = HandEvent::SINGLE;
        if (frame.hands().count() == 2)
        {
            Hand leftHand = frame.hands().leftmost();
            float radius = leftHand.sphereRadius();
            // If left hand is visible, select multiple items at a time
            if ( radius >= RELEASE_TRESHOLD )
                selectionMode_ = HandEvent::MULTIPLE;

            //hand pitch controls zoom/scroll in the view
            float pitch = leftHand.direction().pitch();
            pitch = pitch*180/PI;
            zoomFactor_ = 0;
            if (pitch <= -ANGLE_ZOOM_TRESHOLD)
            {
                zoomFactor_ = ZOOM_FACTOR*(pitch + ANGLE_ZOOM_TRESHOLD);
                zoom = true;
            }
            else if ( pitch >= ANGLE_ZOOM_TRESHOLD )
            {
                zoomFactor_ = ZOOM_FACTOR*(pitch - ANGLE_ZOOM_TRESHOLD);
                zoom = true;
            }
        }

        //always send a move event
        moveEvent();
        detectGesture(frame);
        if ( zoom )
        {
            zoomEvent();
            zoom = false;
        }
        if ( grabbing_ && handState_ == OPEN )
        {
            openEvent();
            grabbing_ = false;
        }
    }
}

void LeapListener::detectGesture(const Frame& pFrame)
{
    GestureList list = pFrame.gestures();
    if (list.count() > 0 && list[0].isValid() )
    {
        Gesture gest = list[0];
        switch ( gest.type() )
        {
        case Gesture::TYPE_INVALID:
            break;
        case Gesture::TYPE_KEY_TAP:
            if ( gest.hands()[0].id() == rightHand_ )
                clickEvent();
            break;
        case Gesture::TYPE_SWIPE:
            if ( gest.hands()[0].id() == rightHand_
                && gest.state() == Gesture::STATE_STOP
                && !swipeTimer_->isActive() )
            {
                SwipeGesture swipe = SwipeGesture(gest);
                // direction almost vertical
                float angle = swipe.direction().angleTo(Vector(0,1,0))*180.0f/PI;
                if (angle <= 20.0f)
                {
                    swipeEvent();
                    swipeTimer_->start();
                }
            }
            break;
        case Gesture::TYPE_CIRCLE:
            if ( gest.hands()[0].id() == rightHand_ &&
                 gest.state() == Gesture::STATE_STOP )
                circleEvent();
            break;
        default:
            break;
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
        // if you leave an item with closed hand = you grab it
        if (handState_ == CLOSE && trackedItem_ != -1)
        {
            grabbing_ = true;
            grabEvent();
        }
        trackPrevious_ = false;
        trackedItem_ = pNewItem;
    }
}

void LeapListener::openEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Opened, rPos_, trackedItem_);
        QApplication::postEvent(receiver_, event);
    }
}

void LeapListener::closeEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Closed, rPos_, trackedItem_);
        QApplication::postEvent(receiver_,event);
    }
}

void LeapListener::zoomEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Zoom, rPos_, trackedItem_, selectionMode_, zoomFactor_);
        QApplication::postEvent(receiver_,event);
    }
}

void LeapListener::clickEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Clicked, rPos_, trackedItem_, selectionMode_);
        QApplication::postEvent(receiver_,event);
    }
}

void LeapListener::doubleClickEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::DoubleClicked, rPos_, trackedItem_, selectionMode_);
        QApplication::sendEvent(receiver_,event);
    }
}

void LeapListener::swipeEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Swiped, rPos_, trackedItem_);
        QApplication::postEvent(receiver_,event);
    }
}

void LeapListener::grabEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Grabbed, rPos_, trackedItem_);
        QApplication::postEvent(receiver_, event);
    }
}

void LeapListener::moveEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Moved, rPos_);
        QApplication::postEvent(receiver_, event);
    }
}

void LeapListener::circleEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Circle, rPos_);
        QApplication::postEvent(receiver_, event);
    }
}
