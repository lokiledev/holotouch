#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QImage>
#include <QList>

#include "leapmotion/Leap.h"

#include "glview.h"
#include "tracking_defines.h"

#define NB_TEXTURE 2
using namespace Leap;

class glWidget : public Glview, public Leap::Listener
{
    Q_OBJECT
public:
    //typedef for textures management
    typedef enum {CRATE, METAL,NONE = -1} texId_t;

    //simple way of describing a cube/item
    struct cube_t {
        float x_;
        float y_;
        float z_;
        float size_;
        texId_t texture_;
        bool selected_;
        bool drawn_;
        //constructor
        cube_t(float pSize = 1.0f, texId_t pText = CRATE);
    };

private:
    GLuint texture_[NB_TEXTURE];
    //head positions in cm relative to screen center.
    head_t head_;
    Leap::Vector palmPos_;
    QList<cube_t> cubeList_;
    int gridSize_;
    float spacing_;

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

    void loadTexture(QString textureName, texId_t pId);
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
    void drawCube(cube_t pCube);
    void drawCurrentGrid();

private:
    void generateCubes(texId_t pTexture, int pNbCubes);
    void computeGrid(float pSpacing);
    int closestCube(float pTreshold);
    void handleSelection();

signals:

public slots:
    void slotNewHead(head_t pPos);
    void slotMoveHead(int pAxis, float pDelta);
    void slotPalmPos(Leap::Vector pPos);
};

#endif // GLWIDGET_H
