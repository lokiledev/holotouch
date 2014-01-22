#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <QCoreApplication>
#include "head_tracking/facedetect.h"

using namespace std;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    int x1,y1,x2,y2;
    int lissage = 1;
    int smooth = 0;
    int delay = 200;
    int opt_scale = 1;
    char key;
    head_t head;
    while(1)
    {
       headtrack(&x1, &y1, &x2, &y2,
                  lissage,
                  smooth,
                  delay,
                  opt_scale);
        cout<<"rect:"<<x1<<y1<<x2<<y2<<std::endl;
        WTLeeTrackPosition (&head,
                                 x1,
                                 y1,
                                 x2,
                                 y2,DEPTH_ADJUST);
        cout<<"head xyz: ("<<head.x<<", "<<head.y<<", "<<head.z<<std::endl;
        key = cvWaitKey(10); //Capture Keyboard stroke
        if ( key  == 27){
            break; //If you hit ESC key loop will break.
        }
    }
    cvDestroyWindow("Camera_Output"); //Destroy Window
    return a.exec();
}
