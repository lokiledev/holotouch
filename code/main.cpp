#include <iostream>
#include <QApplication>

#include "head_tracking/facetrack.h"
#include "mainwindow.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainwindow window;
    try
    {
        window.init();
        window.show();
    }
    catch (string s)
    {
        cerr<<"exception: "<<s<<endl;
    }
    catch (...)
    {
        cerr<<"Something went wrong"<<endl;
    }
    //tracker.~Facetrack();
    return a.exec();
}
