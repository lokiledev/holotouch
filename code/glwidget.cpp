#include "glwidget.h"

#include <math.h>
#include <GL/glu.h>
#include <iostream>

#define BOX_SIZE 5.0f //the grid is always inside the box

#define SCALE_FACTOR_XY 20.0f
#define Z_OFFSET 100.0f
#define Z_SCALE_FACTOR 20.0f
#define Y_OFFSET 200.0f //to scale leapmotion to our view

#define SELECT_TRESHOLD 70.0f //hand hopening in mm
#define RELEASE_TRESHOLD 90.0f //hand opening in mm
#define DEFAULT_SPACING 2.0f

#define HOLD_TIME 30 //nb of frame with hand closed

glWidget::cube_t::cube_t(const QString& pName, float pSize, texId_t pText)
    :x_(0),
     y_(0),
     z_(0),
     size_(pSize),
     sizeOffset_(0),
     texture_(pText),
     selected_(false),
     drawn_(false),
     fileName_(pName)
{
}

glWidget::glWidget(QWidget *parent) :
    Glview(60,parent),
    select_(false),
    gridSize_(0),
    spacing_(DEFAULT_SPACING),
    handState_(OPEN)
{
    head_.x = 0.0;
    head_.y = 0.0;
    head_.z = 5.0;
    palmPos_.x = 0.0f;
    palmPos_.y = 0.0f;
    palmPos_.z = 5.0f;
    setCursor(Qt::BlankCursor);
    generateCubes(CRATE,5*5*5);
    //loadFolder();
}

void glWidget::initializeGL()
{
    loadTexture("../code/ressources/box.png", CRATE);
    loadTexture("../code/ressources/metal.jpg", METAL);
    loadTexture("../code/ressources/Folder.png", FOLDER);
    loadTexture("../code/ressources/music.png", MUSIC);
    loadTexture("../code/ressources/picture.png", PICTURE);
    loadTexture("../code/ressources/text.png", TEXT);

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
    computeGrid();
    handleSelection();
    drawCurrentGrid();
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

    if (frame.hands().count() == 1)
    {
        Hand hand = frame.hands()[0];
        Vector pos = hand.palmPosition();
        //closed hand hard to detect
        // closed = select cube
        float handOpening = 0;
        if ( hand.fingers().isEmpty() )
            handOpening = 0;
        else
            handOpening = hand.sphereRadius();
        static int countClose = 0;
        static int countUp = 0;
        switch(handState_)
        {
        case OPEN:
            countUp++;
            select_ = false;
            if (handOpening <= SELECT_TRESHOLD && countUp >= HOLD_TIME)
            {
                handState_ = CLOSE;
                countUp = 0;
            }
            break;
        case CLOSE:
            countClose++;
            if ( countClose >= HOLD_TIME || handOpening >= RELEASE_TRESHOLD )
            {
                handState_ = OPEN;
                select_ = true;
                countClose = 0;
                slotSelect();
            }
            break;
        default:
            select_ = false;
            break;
        }


        //adjust to our view coordinates
        palmPos_.x = pos.x/SCALE_FACTOR_XY;
        palmPos_.y = (pos.y - Y_OFFSET)/SCALE_FACTOR_XY;
        palmPos_.z = (pos.z - Z_OFFSET)/Z_SCALE_FACTOR;

        //compensate head position to align view and movement
        palmPos_.x -= head_.x;
        palmPos_.y -= head_.y;
    }
}

//helper function, loads a texture and assign it to an enum value
//to help retrieve it later
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



//Draw 6 squares and apply the texture on each: absolute coordinates for the center
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

//Draw a 3D grid composed of L*H cubes of size CubeZise spaced by pSpacing
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

//overloaded function for ease of use
void glWidget::drawCube(const cube_t& pCube)
{
    drawCube(pCube.texture_,
             pCube.x_,
             pCube.y_,
             pCube.z_,
             pCube.size_ + pCube.sizeOffset_);
    if (pCube.fileName_.size() > 0)
    {
        //text is masked by cubes,
        //draw under to see it clearly
        renderText(pCube.x_ - pCube.size_/2.0f,
                   pCube.y_ - pCube.size_,
                   pCube.z_ + pCube.size_/2.0f,
                   pCube.fileName_);
    }
}

//draw a cube where the middle of the palm is
void glWidget::drawPalmPos()
{
    //normalize leap coordinates to our box size
   drawCube(METAL,
            palmPos_.x ,
            palmPos_.y,
            palmPos_.z, 0.5f);
}

//init the view with a certain amount of cubes
void glWidget::generateCubes(texId_t pTexture, int pNbCubes)
{
    cubeList_.clear();
    for (int i=0; i<pNbCubes; ++i)
    {
        cube_t cube("",spacing_/3.0f, pTexture);
        cubeList_.append(cube);
    }
}

/*generate a cubic grid from cubeList_
 *the grid is always in a box of BOX_SIZE
 * so the more cubes, the more smaller it appears
 */
void glWidget::computeGrid()
{
    int nCube = std::roundf(std::cbrt(cubeList_.size()));

    //number of cube per dimension
    gridSize_ = nCube;

    //distance between 2 cube's centers
    spacing_ = BOX_SIZE/nCube;

    //apparent size of the cube
    float size = spacing_/2;

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
                    cubeList_[i].size_ = size;
                    cubeList_[i].x_ = x*spacing_ - offset;
                    cubeList_[i].y_ = y*spacing_ - offset;
                    cubeList_[i].z_ = -z*spacing_;
                }
            }
        }
    }
}

//update the view, draw cubes with absolute center coordinates
//then center the camera
void glWidget::drawCurrentGrid()
{
    QList<cube_t>::iterator it;
    for (it = cubeList_.begin(); it != cubeList_.end(); it++)
        drawCube(*it);
}

//find the closest cube from the palm center
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

/*Detect if a cube needs to be selected
 *perform growing cube animation on each
 *selected cube
 */
void glWidget::handleSelection()
{
    //growing animation on selected cubes
    for (QList<cube_t>::iterator it = cubeList_.begin(); it != cubeList_.end(); it++)
      {
        if ( it->selected_ )
        {
             if(it->sizeOffset_ <= it->size_)
            {
                it->sizeOffset_ += it->size_/10;
            }
        }
        else if ( it->sizeOffset_ > 0)
        {
            it->sizeOffset_ -= it->size_/20;
            if ( it->sizeOffset_ <= 0 )
                it->sizeOffset_ = 0;
        }
    }
}

//generate the view items from the files in a folder
void glWidget::loadFolder(const QDir& pFolder)
{
    fileExplorer_= pFolder;
    QFileInfoList fileList = fileExplorer_.entryInfoList();
    QFileInfoList::iterator it;
    cubeList_.clear();
    for( it = fileList.begin(); it != fileList.end(); it++)
    {
        //TODO: change texture according to file extension

        texId_t texture = CRATE;
        if ( it->isDir() )
            texture = FOLDER;
        else
        {
            //TODO: add better management
            QString ext = it->suffix();
            if ( ext == "png" ||
                 ext == "jpg" ||
                 ext == "bmp")
                texture = PICTURE;
            else if ( ext == "mp3" ||
                      ext == "wav" ||
                      ext == "ogg" ||
                      ext == "flac")
                texture = MUSIC;
           else if ( ext == "txt" ||
                     ext == "sh" ||
                     ext == "cpp" ||
                     ext == "py" )
                texture = TEXT;
        }
        //create new cube, size doesn't matter, recomputed each time
        cube_t item(it->fileName(), 1.0f, texture);
        cubeList_.append(item);
    }
}

//update the camera position
void glWidget::slotNewHead(head_t pPos)
{
    /*We inverse axes to compensate head position relative
     * to the cube.
     */
    head_.x = -pPos.x;
    head_.y = -pPos.y;
    head_.z =  pPos.z;
}

//move slightly the camera, via keyboard commands for example
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


//called when select gesture is made
void glWidget::slotSelect(void)
{
    int cube = closestCube(spacing_);

    //hand is on a single cube
    if (cube != -1 )
    {
        //change state of given cube
        cubeList_[cube].selected_ = !cubeList_[cube].selected_;
    }
    //hand out of grid ====> release everything
    else
    {
        for(int i = 0; i < cubeList_.size(); i++)
        {
            if (i != cube)
                cubeList_[i].selected_ = false;
        }
    }
}

//Not used
void glWidget::slotPalmPos(Vector pPos)
{
    palmPos_ = pPos;
}
