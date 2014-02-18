#include "leapmotion/LeapListener.h"
#include "leapmotion/HandEvent.h"

#include <QApplication>
#include <QDebug>

// adapt the leapmotion coordinates to our application coordinates
#define SCALE_FACTOR_XY 30.0f
#define Z_OFFSET 200.0f
#define Z_SCALE_FACTOR 20.0f
#define Y_OFFSET 200.0f

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
        static bool clicked = false;
        static bool zoom = false;
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
            // If left hand is visible, select multiple items at a time
            if ( radius <= SELECT_TRESHOLD )
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
        if ( detectSwipe(frame) )
            swipeEvent();
        if ( zoom )
            zoomEvent();
            zoom = false;
        if ( clicked )
        {
            clickEvent();
            clicked = false;
        }
        else if ( handState_ == CLOSE )
            closeEvent();
        else if ( handState_ == OPEN )
            openEvent();

    }
}

bool LeapListener::detectSwipe(const Frame& pFrame)
{
    bool ok = false;
    GestureList list = pFrame.gestures();
    if (list.count() > 0 && !swipeTimer_->isActive() )
    {
        Gesture gest = list[0];
        //detect end of gesture
        if ( (gest.type() == Gesture::TYPE_SWIPE)
             && gest.hands()[0].isValid()
             && gest.hands()[0].id() == rightHand_
             && gest.state() == Gesture::STATE_STOP )
        {
            SwipeGesture swipe = SwipeGesture(gest);
            // direction almost vertical
            float angle = swipe.direction().angleTo(Vector(0,1,0))*180.0f/PI;
            if (angle <= 20.0f)
            {
                ok = true;
                swipeTimer_->start();
            }
        }
    }
    return ok;
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

void LeapListener::openEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Opened, rPos_, trackedItem_);
        QApplication::sendEvent(receiver_,event);
    }
}

void LeapListener::closeEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Closed, rPos_, trackedItem_);
        QApplication::sendEvent(receiver_,event);
    }
}

void LeapListener::zoomEvent()
{
    if ( receiver_ )
    {
        HandEvent* event = 0;
        event = new HandEvent(HandEvent::Zoom, rPos_, trackedItem_, selectionMode_, zoomFactor_);
        QApplication::sendEvent(receiver_,event);
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


