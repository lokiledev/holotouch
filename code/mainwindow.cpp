#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QMenu>
#include <QDialog>

#define DELAY_FPS 50

mainwindow::mainwindow(QWidget *parent) :
    QMainWindow(parent),
    webcamView_(0),
    timer_(0),
    menu_(0),
    glView_(0)
{
}

 mainwindow::~mainwindow()
 {
     //stop tracking
    controller_.removeListener(*glView_);
 }

void mainwindow::init(void)
{

    QHBoxLayout* hLayout = new QHBoxLayout();

    webcamView_ = new QLabel("Face View",this);
    hLayout->addWidget(webcamView_);
    webcamView_->hide();

    tracker_.init();
    setWindowTitle("Holotouch");
    QWidget::showMaximized();
    webcamView_->setBackgroundRole(QPalette::Base);
    webcamView_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    webcamView_->setScaledContents(true);
    timer_ = new QTimer(this);

    glView_ = new glWidget(this);
    hLayout->addWidget(glView_);

    menu_ = menuBar()->addMenu("&Display");
    QAction* actionStart = new QAction("Start/Stop", this);
    QAction* actionHide = new QAction("Hide Webcam", this);

    menu_->addAction(actionStart);
    menu_->addAction(actionHide);

    connect(actionStart, SIGNAL(triggered()), this, SLOT(slotStart()));
    connect(actionHide, SIGNAL(triggered()), webcamView_, SLOT(hide()));

    connect(timer_, SIGNAL(timeout()), this, SLOT(slotGetNewFrame()));
    connect(this,SIGNAL(signalNewFrame(QPixmap )), this, SLOT(slotUpdateFrame(QPixmap)));

    QWidget* centerWidget = new QWidget(this);
    centerWidget->setLayout(hLayout);
    setCentralWidget(centerWidget);

    connect(&tracker_, SIGNAL(signalNewHeadPos(head_t)), glView_, SLOT(slotNewHead(head_t)));

    controller_.addListener(*glView_);
    //timer_->start(DELAY_FPS);
}

void mainwindow::slotStart()
{
    if (timer_->isActive())
        timer_->stop();
    else
        timer_->start(DELAY_FPS);
}

void mainwindow::slotGetNewFrame()
{
    tracker_.getNewImg();
    tracker_.detectHead();
    if (tracker_.isNewFace())
    {
        tracker_.drawFace();
        imgWebcam_ = tracker_.getPixmap();
        emit signalNewFrame(imgWebcam_);
    }
}

void mainwindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_Escape:
            if (isFullScreen())
            {
                showMaximized();
                menuBar()->show();
            }
            else
                close();
            break;
        case Qt::Key_F:
            menuBar()->hide();
            showFullScreen();
            break;
        case Qt::Key_Z:
            glView_->slotMoveHead(1, 0.05);
            break;
        case Qt::Key_S:
            glView_->slotMoveHead(1, -0.05);
            break;
        case Qt::Key_Q:
            glView_->slotMoveHead(0, -0.05);
            break;
        case Qt::Key_D:
            glView_->slotMoveHead(0, 0.05);
            break;
        case Qt::Key_A:
            glView_->slotMoveHead(2, -0.05);
            break;
        case Qt::Key_E:
            glView_->slotMoveHead(2, 0.05);
            break;
        case Qt::Key_H:
            webcamView_->isHidden()? webcamView_->show() : webcamView_->hide();
            break;
        case Qt::Key_Space:
            slotStart();
            break;
    }
}


void mainwindow::slotUpdateFrame(QPixmap pNewFrame)
{
    //display webcam image in label,
    //only if it's visible to save processing time
    if (webcamView_->isVisible() )
        webcamView_->setPixmap(pNewFrame.scaled(webcamView_->width(), webcamView_->height(),Qt::KeepAspectRatio));
}

void mainwindow::slotAbout()
{
    //TODO: about dialog
}
