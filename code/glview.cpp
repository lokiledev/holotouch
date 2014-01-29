#include "glview.h"

Glview::Glview(int framesPerSecond, QWidget *parent)
    : QGLWidget(parent)
{
    if(framesPerSecond == 0)
        t_Timer = NULL;
    else
    {
        int seconde = 1000; // 1 seconde = 1000 ms
        int timerInterval = seconde / framesPerSecond;
        t_Timer = new QTimer(this);
        connect(t_Timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()));
        t_Timer->start( timerInterval );
    }
}

void Glview::timeOutSlot()
{
    updateGL();
}

