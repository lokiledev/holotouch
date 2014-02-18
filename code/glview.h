#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H

#include <QtOpenGL>
#include <QGLWidget>

class Glview : public QGLWidget
{
    Q_OBJECT

protected:
    QTimer *timer_;

public:
    Glview(int framesPerSecond = 0, QWidget *parent = 0);
    virtual void initializeGL() = 0;
    virtual void resizeGL(int width, int height) = 0;
    virtual void paintGL() = 0;

public slots:
    void timeOutSlot();
};


#endif // GLVIEWWIDGET_H
