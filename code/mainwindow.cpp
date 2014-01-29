#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#define DELAY_FPS 50

mainwindow::mainwindow(QWidget *parent) :
    QMainWindow(parent),
    webcamView_(0),
    timer_(0),
    startBtn_(0)
{
}

void mainwindow::init(void)
{
    webcamView_ = new QLabel("Face View",this);
    setCentralWidget(webcamView_);
    tracker_.init();
    setWindowTitle("Holotouch");
    QWidget::showMaximized();
    webcamView_->setBackgroundRole(QPalette::Base);
    webcamView_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    webcamView_->setScaledContents(true);
    timer_ = new QTimer(this);

    startBtn_ = new QPushButton("Start processing", this);
    setMenuWidget(startBtn_);
    connect(startBtn_, SIGNAL(clicked()),this, SLOT(slotStart()));
    connect(timer_, SIGNAL(timeout()), this, SLOT(slotGetNewFrame()));
    connect(this,SIGNAL(signalNewFrame(QPixmap )), this, SLOT(slotUpdateFrame(QPixmap)));
}

void mainwindow::mainLoop(void)
{
    while(1)
    {
        tracker_.getNewImg();
        tracker_.detectHead();
        tracker_.drawFace();
        //tracker_.showFace();
        imgWebcam_ = tracker_.getPixmap();
        webcamView_->setPixmap(imgWebcam_.scaled(webcamView_->width(), webcamView_->height(),Qt::KeepAspectRatio));
        tracker_.getCoordinates();
        tracker_.WTLeeTrackPosition(DEPTH_ADJUST/10000.0);
        if( waitKey( 10 ) >= 0 )
        {
            break;
        }
    }
}

void mainwindow::slotStart()
{
    timer_->start(DELAY_FPS);
}

void mainwindow::slotGetNewFrame()
{
    tracker_.getNewImg();
    tracker_.detectHead();
    tracker_.drawFace();
    imgWebcam_ = tracker_.getPixmap();
    tracker_.getCoordinates();
    tracker_.WTLeeTrackPosition(DEPTH_ADJUST/10000.0);
    emit signalNewFrame(imgWebcam_);
}

void mainwindow::slotUpdateFrame(QPixmap pNewFrame)
{
    webcamView_->setPixmap(pNewFrame.scaled(webcamView_->width(), webcamView_->height(),Qt::KeepAspectRatio));
}
