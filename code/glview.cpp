#include "glview.h"

Glview::Glview(int framesPerSecond, QWidget *parent)
    : QGLWidget(parent)
{
    if(framesPerSecond == 0)
        timer_ = NULL;
    else
    {
        int seconde = 1000; // 1 seconde = 1000 ms
        int timerInterval = seconde / framesPerSecond;
        timer_ = new QTimer(this);
        timer_->setInterval(timerInterval);
        connect(timer_, SIGNAL(timeout()), this, SLOT(timeOutSlot()));
        timer_->start();
    }
}

void Glview::timeOutSlot()
{
    updateGL();
}

