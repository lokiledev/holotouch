#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glwidget.h"

class QLabel;
class QTimer;
class QMenu;

#include "head_tracking/facetrack.h"

class mainwindow : public QMainWindow
{
    Q_OBJECT

private:
    Facetrack tracker_;
    QLabel* webcamView_;
    QPixmap imgWebcam_;
    QTimer* timer_;
    QMenu* menu_;
    glWidget* glView_;

public:
    mainwindow(QWidget *parent = 0);
    void init(void);
    void mainLoop(void);
    void keyPressEvent( QKeyEvent *keyEvent);

signals:
    void signalNewFrame(QPixmap pNewFrame);
    void signalNewHeadPos(head_t head);

public slots:
    void slotStart();

private slots:
   void slotGetNewFrame();
   void slotUpdateFrame(QPixmap pNewFrame);

};

#endif // MAINWINDOW_H
