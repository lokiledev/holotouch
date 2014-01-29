#include "mainwindow.h"

mainwindow::mainwindow(QWidget *parent) :
    QMainWindow(parent),
    webcamView_(0)
{
}

void mainwindow::init(void)
{
    webcamView_ = new QLabel(this);
    setCentralWidget(webcamView_);
    tracker_.init();
    setWindowTitle("Holotouch");
    QWidget::showMaximized();
    webcamView_->setBackgroundRole(QPalette::Base);
    webcamView_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    webcamView_->setScaledContents(true);
}

void mainwindow::mainLoop(void)
{
    while(1)
    {
        tracker_.getNewImg();
        tracker_.detectHead();
        //tracker_.showFace();
        imgWebcam_ = tracker_.showFace();
        webcamView_->setPixmap(imgWebcam_);
        tracker_.getCoordinates();
        tracker_.WTLeeTrackPosition(DEPTH_ADJUST/10000.0);
        if( waitKey( 10 ) >= 0 )
        {
            break;
        }
    }
}
