#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <QApplication>

//#include "head_tracking/facedetect.h"
#include "head_tracking/facetrack.hpp"

using namespace std;
/*int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    int x1,y1,x2,y2;
    int lissage = 0;
    int smooth = 0;
    int delay = 0;
    double opt_scale = 40.0;
    char key;
    head_t head;
    head.x = 0.0;
    head.y = 0.0;
    head.z = 1.0;
    //cvNamedWindow("Camera_Output", 1); //Create window
    while(1)
    {
       headtrack(&x1, &y1, &x2, &y2,
                  lissage,
                  smooth,
                  delay,
                  opt_scale);
        cout<<"rect:"<<x1<<", "<<y1<<", "<<", "<<x2<<", "<<y2;
        WTLeeTrackPosition (&head,
                                 (float)x1,
                                 (float)y1,
                                 (float)x2,
                                 (float)y2,DEPTH_ADJUST/10000.0);
        cout<<" head xyz: ("<<head.x<<", "<<head.y<<", "<<head.z<<")"<<'\xd';
        key = cvWaitKey(10); //Capture Keyboard stroke
        if ( key  == 27){
            break; //If you hit ESC key loop will break.
        }
    }
    //cvDestroyWindow("Camera_Output"); //Destroy Window
    endThread();
    return a.exec();
}*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Facetrack tracker;
    try
    {
        tracker.init();
        while(1)
        {
            tracker.getNewImg();
            tracker.showImg();
            if( waitKey( 10 ) >= 0 )
            {
                break;
            }
        }
    }
    catch (string s)
    {
        cerr<<"exception: "<<s<<endl;
    }
    catch (...)
    {
        cerr<<"Something went wrong"<<endl;
    }
    tracker.~Facetrack();
    return a.exec();
}
