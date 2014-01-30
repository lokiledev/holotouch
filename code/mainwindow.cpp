#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>

#define DELAY_FPS 50

mainwindow::mainwindow(QWidget *parent) :
    QMainWindow(parent),
    webcamView_(0),
    timer_(0),
    startBtn_(0),
    glView_(0)
{
}

void mainwindow::init(void)
{

    QHBoxLayout* hLayout = new QHBoxLayout();

    webcamView_ = new QLabel("Face View",this);
    hLayout->addWidget(webcamView_);

    tracker_.init();
    setWindowTitle("Holotouch");
    QWidget::showMaximized();
    webcamView_->setBackgroundRole(QPalette::Base);
    webcamView_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    webcamView_->setScaledContents(true);
    timer_ = new QTimer(this);

    glView_ = new glWidget(this);

    hLayout->addWidget(glView_);
    startBtn_ = new QPushButton("Start processing", this);
    setMenuWidget(startBtn_);
    connect(startBtn_, SIGNAL(clicked()),this, SLOT(slotStart()));
    connect(timer_, SIGNAL(timeout()), this, SLOT(slotGetNewFrame()));
    connect(this,SIGNAL(signalNewFrame(QPixmap )), this, SLOT(slotUpdateFrame(QPixmap)));

    QWidget* centerWidget = new QWidget(this);
    centerWidget->setLayout(hLayout);
    setCentralWidget(centerWidget);

    connect(&tracker_, SIGNAL(signalNewHeadPos(head_t)), glView_, SLOT(slotNewHead(head_t)));
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

void mainwindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_F:
            showFullScreen();
            break;
        case Qt::Key_Up:
            if ( glView_ )
                glView_->slotMoveHead(1, 0.5);
            break;
        case Qt::Key_Down:
        if ( glView_ )
            glView_->slotMoveHead(1, -0.5);
            break;
        case Qt::Key_Left:
        if ( glView_ )
            glView_->slotMoveHead(0, -0.5);
            break;
        case Qt::Key_Right:
        if ( glView_ )
            glView_->slotMoveHead(0, 0.5);
            break;
    }
}


void mainwindow::slotUpdateFrame(QPixmap pNewFrame)
{
    webcamView_->setPixmap(pNewFrame.scaled(webcamView_->width(), webcamView_->height(),Qt::KeepAspectRatio));
}
