#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtOpenGL>
#include <QGLWidget>

class glCubes : public QGLWidget
{
    Q_OBJECT
public:
    explicit glCubes(int framesPerSecond = 0, QWidget *parent = 0, char *name = 0);
    virtual void initializeGL() = 0;
    virtual void resizeGL(int width, int height) = 0;
    virtual void paintGL() = 0;
    virtual void keyPressEvent( QKeyEvent *keyEvent );

public slots:
    virtual void timeOutSlot();

private:
    QTimer *t_Timer;

};

#endif // MAINWINDOW_H
