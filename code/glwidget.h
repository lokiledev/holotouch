#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QImage>
#include <QList>
#include <QDir>
#include <QMutex>

#include "leapmotion/Leap.h"

#include "glview.h"
#include "tracking_defines.h"

#define NB_TEXTURE 6
#define BOX_SIZE 5.0f //the grid is always inside the box

using namespace Leap;

class glWidget : public Glview, public Leap::Listener
{
    Q_OBJECT
public:
    //typedef for textures management
    typedef enum {CRATE, METAL, FOLDER, MUSIC, PICTURE, TEXT, NONE = -1} texId_t;

    typedef enum {SINGLE, MULTIPLE} selectMode_t;

    typedef enum {IDLE, EXPAND, COLLAPSE} globalAnimation_t;

    //state machine for clic like gestures
    typedef enum {OPEN,CLOSE} handState_t;

    //simple way of describing a cube/item
    struct item_t {
        float x_;
        float y_;
        float z_;
        float size_;
        float sizeOffset_;
        texId_t texture_;
        bool selected_;
        bool drawn_;
        QString fileName_;
        //constructor
        item_t(const QString& pName = "", float pSize = 1.0f, texId_t pText = CRATE);
    };

private:
    GLuint texture_[NB_TEXTURE];
    //head positions in cm relative to screen center.
    head_t head_;
    Leap::Vector palmPos_;
    QList<item_t> itemList_;
    float boxSize_;
    int gridSize_;
    float spacing_;
    QDir fileExplorer_;
    handState_t handState_;
    selectMode_t selectionMode_;
    globalAnimation_t currentAnim_;

    mutable QMutex mutexList_;

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
    void drawCube(const item_t& pCube);
    void drawTile(texId_t PtextureId,
                  float pCenterX,
                  float pCenterY,
                  float pCenterZ,
                  float pSize);
    void drawTile(const item_t& pItem);
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
    void drawCurrentGrid();
    void reloadFolder();
    void changeDirectory(const QString& pFolder);

private:
    void generateCubes(texId_t pTexture, int pNbCubes);
    void computeGrid(float pPboxSize = BOX_SIZE);
    int closestItem(float pTreshold);
    void handleSelection();

signals:

public slots:
    void slotNewHead(head_t pPos);
    void slotMoveHead(int pAxis, float pDelta);
    void slotPalmPos(Leap::Vector pPos);

private slots:
    void slotSelect(void);
};

#endif // GLWIDGET_H
