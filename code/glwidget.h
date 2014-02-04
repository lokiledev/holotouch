#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QImage>

#include "leapmotion/Leap.h"

#include "glview.h"
#include "tracking_defines.h"

using namespace Leap;

class glWidget : public Glview, public Leap::Listener
{
    Q_OBJECT
public:
    typedef enum {CRATE,NONE = -1} texId_t;
private:
    GLuint texture_[1];
    //head positions in cm relative to screen center.
    head_t head_;
    Leap::Vector palmPos_;

public:
    glWidget(QWidget *parent = 0);

    //opengl functions
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    //leap listener functions
    void onInit(const Controller&);
    void onConnect(const Controller&);
    void onDisconnect(const Controller&);
    void onExit(const Controller&);
    void onFrame(const Controller&);

    void loadTexture(QString textureName);
    void drawCube(texId_t PtextureId,
                  float pCenterX,
                  float pCenterY,
                  float pCenterZ,
                  float pSize);
    void drawCube2DGrid(texId_t pTexture,
                        float pSpacing,
                        float pCubeSize,
                        int pL,
                        int pH);
    void drawCube3DGrid(texId_t pTexture,
                                  float pSpacing,
                                  float pCubeSize,
                                  int pL,
                                  int pH,
                                  int pW);
    void drawPalmPos();

signals:

public slots:
    void slotNewHead(head_t pPos);
    void slotMoveHead(int pAxis, float pDelta);
    void slotPalmPos(Leap::Vector pPos);
};

#endif // GLWIDGET_H
