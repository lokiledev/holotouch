#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glview.h"

class glWidget : public Glview
{
    Q_OBJECT
public:
    glWidget(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

signals:

public slots:

};

#endif // GLWIDGET_H
