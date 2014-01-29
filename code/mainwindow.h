#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "head_tracking/facetrack.h"

class mainwindow : public QMainWindow
{
    Q_OBJECT

private:
    Facetrack tracker_;
    QLabel* webcamView_;
    QPixmap imgWebcam_;

public:
    mainwindow(QWidget *parent = 0);
    void init(void);
    void mainLoop(void);

signals:

public slots:

};

#endif // MAINWINDOW_H
