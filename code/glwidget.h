#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QImage>
#include <QList>
#include <QDir>
#include <QMutex>

#include "leapmotion/LeapListener.h"

#include "glview.h"
#include "tracking_defines.h"

#define NB_TEXTURE 7
#define BOX_SIZE 5.0f //the grid is always inside the box

using namespace Leap;

class GlWidget : public Glview
{
    Q_OBJECT
public:
    //typedef for textures management
    typedef enum {CRATE,
                  METAL,
                  FOLDER,
                  MUSIC,
                  PICTURE,
                  TEXT,
                  VIDEO,
                  NONE = -1} texId_t;

    typedef enum {IDLE, EXPAND, COLLAPSE} globalAnimation_t;

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
    LeapListener leapListener_;
    Leap::Controller controller_;

    GLuint texture_[NB_TEXTURE];

    //head positions in cm relative to screen center.
    head_t head_;
    Leap::Vector palmPos_;
    QList<item_t> itemList_;
    HandEvent::Selection_t selectionMode_;
    float boxSize_;
    int gridSize_;
    float spacing_;

    QDir fileExplorer_;
    globalAnimation_t currentAnim_;

    mutable QMutex mutexList_;

public:
    GlWidget(QWidget *parent = 0);
    ~GlWidget();

    //opengl functions
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

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
    void customEvent(QEvent* pEvent);

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
    void slotSelect(int pItem = -1);
};

#endif // GLWIDGET_H
