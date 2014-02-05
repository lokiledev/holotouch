#include "glwidget.h"

#include <math.h>
#include <GL/glu.h>
#include <iostream>

#define SCALE_FACTOR_XY 20.0f
#define Z_OFFSET 200.0f
#define Z_SCALE_FACTOR 10.0f
#define Y_OFFSET 200.0f
#define SELECT_TRESHOLD 65.0f
#define RELEASE_TRESHOLD 100.0f
#define DEFAULT_SPACING 2.0f


glWidget::cube_t::cube_t(float pSize, texId_t pText)
    :x_(0),
     y_(0),
     z_(0),
     size_(pSize),
     sizeOffset_(0),
     texture_(pText),
     selected_(false),
     drawn_(false)
{
}

glWidget::glWidget(QWidget *parent) :
    Glview(60,parent),
    handOpening_(10.0f),
    selectMove_(false),
    gridSize_(0),
    spacing_(DEFAULT_SPACING)
{
    head_.x = 0.0;
    head_.y = 0.0;
    head_.z = 5.0;
    palmPos_.x = 0.0f;
    palmPos_.y = 0.0f;
    palmPos_.z = 5.0f;
    setCursor(Qt::BlankCursor);
    generateCubes(CRATE,125);
}

void glWidget::initializeGL()
{
    loadTexture("../code/ressources/box.png", CRATE);
    loadTexture("../code/ressources/metal.jpg", METAL);

    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void glWidget::resizeGL(int width, int height)
{
    if(height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 1.0f, -100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void glWidget::paintGL()
{
    // ============================
    // Render Scene
    // ============================

    // clear the back buffer and z buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // disable lighting
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    gluLookAt(head_.x,head_.y,head_.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,0.0f);

    // Objects
    drawPalmPos();
    computeGrid(spacing_);
    handleSelection();
    drawCurrentGrid();
    //drawCube3DGrid(CRATE, 1.0f, 1.0f, 5, 5, 5);
}


void glWidget::onInit(const Controller& controller) {
    Q_UNUSED(controller);
    std::cout << "Initialized" << std::endl;
}

void glWidget::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void glWidget::onDisconnect(const Controller& controller) {
    //Note: not dispatched when running in a debugger.
    Q_UNUSED(controller);
    std::cout << "Disconnected" << std::endl;
}

void glWidget::onExit(const Controller& controller) {
    Q_UNUSED(controller);
    std::cout << "Exited" << std::endl;
}

void glWidget::onFrame(const Controller& controller) {
    Q_UNUSED(controller);
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();
    /*std::cout << ", hands: " << frame.hands().count()
               <<", palm pos: "<< frame.hands()[0].palmPosition()<<std::endl;
    */

    selectMove_ = false;
    if (frame.gestures().count() > 0)
    {
        Gesture up = frame.gestures()[0];
        if (up.type() == Gesture::TYPE_SWIPE)
            selectMove_ = true;
    }

    if (frame.hands().count() == 1)
    {
        Hand hand = frame.hands()[0];
        Vector pos = hand.palmPosition();
        if ( hand.fingers().isEmpty() )
            handOpening_ = 0;
        else
            handOpening_ = hand.sphereRadius();

        //adjust to our view coordinates
        palmPos_.x = pos.x/SCALE_FACTOR_XY;
        palmPos_.y = (pos.y - Y_OFFSET)/SCALE_FACTOR_XY;
        palmPos_.z = (pos.z - Z_OFFSET)/Z_SCALE_FACTOR;

        //compensate head position to align view and movement
        palmPos_.x -= head_.x;
        palmPos_.y -= head_.y;
    }
}


void glWidget::loadTexture(QString textureName, texId_t pId)
{
    QImage qim_Texture;
    QImage qim_TempTexture;
    qim_TempTexture.load(textureName);
    qim_Texture = QGLWidget::convertToGLFormat( qim_TempTexture );
    glGenTextures( 1, &texture_[pId] );
    glBindTexture( GL_TEXTURE_2D, texture_[pId] );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, qim_Texture.width(), qim_Texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qim_Texture.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void glWidget::slotNewHead(head_t pPos)
{
    /*We inverse axes to compensate head position relative
     * to the cube.
     */
    head_.x = -pPos.x;
    head_.y = -pPos.y;
    head_.z = pPos.z;
}

void glWidget::slotMoveHead(int pAxis, float pDelta)
{
    switch(pAxis)
    {
        case 0:
           head_.x += pDelta;
           break;
        case 1:
            head_.y += pDelta;
            break;
        case 2:
            head_.z += pDelta;
        default:
            break;
    }
}

void glWidget::drawCube(texId_t PtextureId, float pCenterX, float pCenterY,float pCenterZ, float pSize)
{

    float half = pSize/2;
    glBindTexture(GL_TEXTURE_2D, texture_[PtextureId]);

    glBegin(GL_QUADS);
    // front fixed Z near (positive)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ-half);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ-half);

    // back fixed z far (negative)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+half);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ+half);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ+half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+half);

    // top fixed Y up
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half,  pCenterY+half, pCenterZ-half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half,  pCenterY+half, pCenterZ+half);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half,  pCenterY+half, pCenterZ+half);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half,  pCenterY+half, pCenterZ-half);

    // bottom fixed Y down
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+half);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+half);

    // Right fixed X (positive)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ-half);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ+half);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+half);

    // Left fixed x negative
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-half);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+half);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ+half);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ-half);
    glEnd();
}

//Draw a 2D grid composed of L*H cubes of size CubeZise spaced by pSpacing
void glWidget::drawCube2DGrid(texId_t pTexture,float pSpacing, float pCubeSize, int pL,int pH)
{
    glTranslatef(-(pSpacing+pCubeSize)*(pL-1)/2,(pSpacing+pCubeSize)*(pH-1)/2,0);
    for(int i = 0; i < pH; ++i)
    {
        for(int j = 0; j < pL; ++j)
        {
            drawCube(pTexture,0,0,0,pCubeSize);
            //draw next cube to the right (greater X)
            glTranslatef(pSpacing+pCubeSize,0,0);
        }
        //go back at the start of the line and draw downards y
        glTranslatef(-(pSpacing + pCubeSize)*pL,-(pSpacing+pCubeSize),0);
    }
}

//Draw a 2D grid composed of L*H cubes of size CubeZise spaced by pSpacing
void glWidget::drawCube3DGrid(texId_t pTexture,
                              float pSpacing,
                              float pCubeSize,
                              int pL,
                              int pH,
                              int pW)
{
    glTranslatef(-(pSpacing+pCubeSize)*(pL-1)/2,
                 (pSpacing+pCubeSize)*(pH-1)/2,
                 -(pSpacing+pCubeSize)*(pW-1)/2);
    for(int z = 0; z < pW; z++)
    {
        for(int y = 0; y < pH; ++y)
        {
            for(int x = 0; x < pL; ++x)
            {
                drawCube(pTexture,0,0,0,pCubeSize);
                //draw next cube to the right (greater X)
                glTranslatef(pSpacing+pCubeSize,0,0);
            }
            //go back at the start of the line and draw downards y
            glTranslatef(-(pSpacing + pCubeSize)*pL,-(pSpacing+pCubeSize),0);
        }
        glTranslatef(0,(pSpacing+pCubeSize)*pH,-(pSpacing+pCubeSize));
    }
}

void glWidget::drawCube(cube_t pCube)
{
    drawCube(pCube.texture_,
             pCube.x_,
             pCube.y_,
             pCube.z_,
             pCube.size_ + pCube.sizeOffset_);
}

void glWidget::drawPalmPos()
{
    //normalize leap coordinates to our box size
   drawCube(METAL,
            palmPos_.x ,
            palmPos_.y,
            palmPos_.z, 0.5f);
}

void glWidget::generateCubes(texId_t pTexture, int pNbCubes)
{
    cubeList_.clear();
    for (int i=0; i<pNbCubes; ++i)
    {
        cube_t cube(spacing_/3.0f, pTexture);
        cubeList_.append(cube);
    }
}

//generate a cubic grid from cubeList_
//pSpacing is between each cube center
void glWidget::computeGrid(float pSpacing)
{
    spacing_ = pSpacing;
    int nCube = std::roundf(std::cbrt(cubeList_.size()));

    //to center front face on (0,0,0)
    float offset = (nCube-1)*spacing_/2;
    for (int z = 0; z <= nCube; z++)
    {
        for (int y = 0; y <= nCube; y++)
        {
            for (int x = 0; x <= nCube; x++)
            {
                int i = z*nCube*nCube + y*nCube + x;
                //avoid overflow
                if (i < cubeList_.size() )
                {
                    cubeList_[i].x_ = x*spacing_ - offset;
                    cubeList_[i].y_ = y*spacing_ - offset;
                    cubeList_[i].z_ = -z*spacing_;
                    if ( !cubeList_[i].selected_ )
                        cubeList_[i].sizeOffset_ = 0;
                }
            }
        }
    }
}

void glWidget::drawCurrentGrid()
{
    QList<cube_t>::iterator it;
    for (it = cubeList_.begin(); it != cubeList_.end(); it++)
        drawCube(*it);
    glTranslatef((gridSize_*spacing_)/2, (gridSize_*spacing_)/2, 0.0f);
}

int glWidget::closestCube(float pTreshold)
{
    float minDist = 1000.0f;
    QList<cube_t>::iterator it;
    int id = -1, i = 0;
    for (it = cubeList_.begin(); it != cubeList_.end(); it++)
    {
        Leap::Vector testV(it->x_,it->y_, it->z_);
        float delta = palmPos_.distanceTo(testV);
        if ((delta <= pTreshold) && (delta <= minDist))
        {
            minDist = delta;
            id = i;
        }
        i++;
    }
    return id;
}

void glWidget::handleSelection()
{
    int cube = closestCube(1.0f);
    if (cube != -1 )
    {
        if ( !cubeList_[cube].selected_  && (handOpening_ <= SELECT_TRESHOLD) )
        {
            cubeList_[cube].selected_ = true;
        }
    }
    else if (handOpening_ >= RELEASE_TRESHOLD)
    {
        for(int i = 0; i < cubeList_.size(); i++)
        {
            if (i != cube)
                cubeList_[i].selected_ = false;
        }
    }
    for (QList<cube_t>::iterator it = cubeList_.begin(); it != cubeList_.end(); it++)
      {
        if ( (it->selected_) && (it->sizeOffset_ <= 1.0f) )
        {
            it->sizeOffset_ += 0.05f;
        }
    }
}

void glWidget::slotPalmPos(Vector pPos)
{
    palmPos_ = pPos;
}
