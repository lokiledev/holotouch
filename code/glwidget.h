#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QImage>

#include "glview.h"

class glWidget : public Glview
{
    Q_OBJECT
private:
    GLuint texture_[1];
    float f_x_;

public:
    glWidget(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void loadTexture(QString textureName);

signals:

public slots:

};

#endif // GLWIDGET_H
