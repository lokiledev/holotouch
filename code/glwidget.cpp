#include "glwidget.h"

#include <math.h>
#include <GL/glu.h>
#include <iostream>

#include <QMutexLocker>

#define SCALE_FACTOR_XY 30.0f
#define Z_OFFSET 100.0f
#define Z_SCALE_FACTOR 40.0f
#define Y_OFFSET 200.0f //to scale leapmotion to our view

#define SELECT_TRESHOLD 60.0f //hand hopening in mm
#define RELEASE_TRESHOLD 80.0f //hand opening in mm
#define DEFAULT_SPACING 2.0f

#define HOLD_TIME 5 //nb of frame with hand closed

glWidget::item_t::item_t(const QString& pName, float pSize, texId_t pText)
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
    boxSize_(BOX_SIZE),
    gridSize_(0),
    spacing_(DEFAULT_SPACING),
    fileExplorer_(QDir::home()),
    handState_(OPEN),
    selectionMode_(SINGLE),
    currentAnim_(IDLE)
{
    head_.x = 0.0;
    head_.y = 0.0;
    head_.z = 5.0;
    palmPos_.x = 0.0f;
    palmPos_.y = 0.0f;
    palmPos_.z = 5.0f;
    setCursor(Qt::BlankCursor);
    //generateCubes(CRATE,5*5*5);
    reloadFolder();
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
    computeGrid(boxSize_);
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
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();

    if (frame.hands().count() >= 1)
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

        //small state machine to detect closed/opened hand
        //use an hysteresis on hand sphere radius
        static int countClose = 0;
        static int countUp = 0;
        switch(handState_)
        {
        case OPEN:
            if (handOpening <= SELECT_TRESHOLD)
            {
                countUp++;
                if ( countUp >= HOLD_TIME )
                {
                    handState_ = CLOSE;
                    countUp = 0;
                }
            }
            else
                countUp = 0;
            break;
        case CLOSE:
            if ( handOpening >= RELEASE_TRESHOLD )
            {
                countClose++;
                if ( countClose >= HOLD_TIME )
                {
                    handState_ = OPEN;
                    countClose = 0;
                    slotSelect();
                }
            }
            else
                countClose = 0;
            break;
        default:
            break;
        }

        //adjust to our view coordinates
        palmPos_.x = pos.x/SCALE_FACTOR_XY;
        palmPos_.y = (pos.y - Y_OFFSET)/SCALE_FACTOR_XY;
        palmPos_.z = (pos.z - Z_OFFSET)/Z_SCALE_FACTOR;

        //compensate head position to align view and movement
        palmPos_.x -= head_.x;
        palmPos_.y -= head_.y;

        selectionMode_ = SINGLE;
        if (frame.hands().count() == 2)
        {
            Hand leftHand = frame.hands().leftmost();
            float radius = leftHand.sphereRadius();
            if ( radius <= SELECT_TRESHOLD )
                selectionMode_ = MULTIPLE;
        }
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

//Draw a tile of pSize, apply texture everywhere (to be enhanced)
void glWidget::drawTile(texId_t PtextureId, float pCenterX, float pCenterY,float pCenterZ, float pSize)
{
    float half = pSize/2;
    float thickness = pSize/8;
    glBindTexture(GL_TEXTURE_2D, texture_[PtextureId]);

    glBegin(GL_QUADS);
    // front fixed Z near (positive)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ-thickness);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ-thickness);

    // back fixed z far (negative)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+thickness);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+thickness);

    // top fixed Y up
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half,  pCenterY+half, pCenterZ-thickness);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half,  pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half,  pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half,  pCenterY+half, pCenterZ-thickness);

    // bottom fixed Y down
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+thickness);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+thickness);

    // Right fixed X (positive)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ-thickness);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX+half, pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX+half, pCenterY-half, pCenterZ+thickness);

    // Left fixed x negative
    glTexCoord2f(0.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ-thickness);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(pCenterX-half, pCenterY-half, pCenterZ+thickness);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ+thickness);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(pCenterX-half, pCenterY+half, pCenterZ-thickness);
    glEnd();
}

void glWidget::drawTile(const item_t& pItem)
{
    drawTile(pItem.texture_,
             pItem.x_,
             pItem.y_,
             pItem.z_,
             pItem.size_ + pItem.sizeOffset_);
    if (pItem.fileName_.size() > 0)
    {
        //text is masked by cubes,
        //draw under to see it clearly
        renderText(pItem.x_ - pItem.size_/2.0f,
                   pItem.y_ - pItem.size_,
                   pItem.z_ + pItem.size_/2.0f,
                   pItem.fileName_);
    }
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
void glWidget::drawCube(const item_t& pCube)
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
            palmPos_.z, BOX_SIZE/(4*gridSize_));
}

//init the view with a certain amount of cubes
void glWidget::generateCubes(texId_t pTexture, int pNbCubes)
{
    itemList_.clear();
    for (int i=0; i<pNbCubes; ++i)
    {
        item_t item("",spacing_/3.0f, pTexture);
        itemList_.append(item);
    }
}

/*generate a cubic grid from itemList_
 *the grid is always in a box of BOX_SIZE
 * so the more cubes, the more smaller it appears
 */
void glWidget::computeGrid(float pBoxSize)
{
    int nbItem = std::roundf(std::cbrt(itemList_.size()));

    //number of cube per dimension
    gridSize_ = nbItem;

    //distance between 2 cube's centers
    spacing_ = pBoxSize/nbItem;

    //apparent size of the cube
    float size = spacing_/2;

    //to center front face on (0,0,0)
    float offset = (nbItem-1)*spacing_/2;
    QMutexLocker locker(&mutexList_);
    for (int z = 0; z <= nbItem; z++)
    {
        for (int y = 0; y <= nbItem; y++)
        {
            for (int x = 0; x <= nbItem; x++)
            {
                int i = z*nbItem*nbItem + y*nbItem + x;
                //avoid overflow
                if (i < itemList_.size() )
                {
                    itemList_[i].size_ = size;
                    itemList_[i].x_ = x*spacing_ - offset;
                    itemList_[i].y_ = y*spacing_ - offset;
                    itemList_[i].z_ = -z*spacing_;
                }
            }
        }
    }
}

//update the view, draw items with absolute center coordinates
void glWidget::drawCurrentGrid()
{
    QList<item_t>::iterator it;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
        drawTile(*it);
        //drawCube(*it);
}

//find the closest cube from the palm center
int glWidget::closestItem(float pTreshold)
{
    float minDist = 1000.0f;
    QList<item_t>::iterator it;
    int id = -1, i = 0;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
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
    for (QList<item_t>::iterator it = itemList_.begin(); it != itemList_.end(); it++)
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

void glWidget::changeDirectory(const QString& pFolder)
{
    bool ok = false;
    if ( pFolder == "..")
    {
        if( fileExplorer_.cdUp() )
            ok = true;
    }
    else if ( fileExplorer_.cd(pFolder) )
        ok = true;
    if ( ok )
        reloadFolder();
}

//generate the view items from the files in a folder
void glWidget::reloadFolder()
{
    //protect access on the datalist
    QMutexLocker locker(&mutexList_);

    qDebug() << "loaded folder: "<< fileExplorer_.path();

    QFileInfoList fileList = fileExplorer_.entryInfoList();
    QFileInfoList::const_iterator it;
    QList<item_t> newList;

    for( it = fileList.cbegin(); it != fileList.cend(); it++)
    {
        //TODO: choose better textures
        texId_t texture = CRATE;
        if ( it->isDir() )
            texture = FOLDER;
        else
        {
            //TODO: add more extensions
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
        item_t item(it->fileName(), 1.0f, texture);
        newList.append(item);
    }
    itemList_ = newList;
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
    int item = closestItem(spacing_);

    //hand is on a single item
    if (item != -1 )
    { 
        if ( selectionMode_ == SINGLE )
        {
            //select item previously not selected
            if ( !itemList_[item].selected_ )
            {
                itemList_[item].selected_ = true;
                for( int i = 0; i < itemList_.size(); i++ )
                {
                    if (i != item)
                        itemList_[i].selected_ = false;
                }
            }
            else //open previously selected item
            {
                if ( item < fileExplorer_.entryInfoList().size() )
                {
                    QFileInfo info =  fileExplorer_.entryInfoList().at(item);
                    if ( info.isDir() )
                    {
                        //reset view to new folder
                        changeDirectory(info.fileName());
                    }
                    else
                    {
                        QDesktopServices::openUrl(
                                    QUrl("file://"+ info.absoluteFilePath()));
                    }
                }
            }

        }
        else if (selectionMode_ == MULTIPLE)
        {
            itemList_[item].selected_ = !itemList_[item].selected_ ;
        }
    }
    //hand out of grid ====> release everything
    else
    {
        for(int i = 0; i < itemList_.size(); i++)
            itemList_[i].selected_ = false;
    }
}

//Not used
void glWidget::slotPalmPos(Vector pPos)
{
    palmPos_ = pPos;
}
