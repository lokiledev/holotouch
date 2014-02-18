#include "glwidget.h"

#include <math.h>
#include <GL/glu.h>
#include <iostream>

#include <QMutexLocker>

#include "leapmotion/HandEvent.h"

#define DEFAULT_SPACING 2.0f
#define GRAB_SCALE 1.1f
GlWidget::item_t::item_t(const QString& pName, float pSize, texId_t pText)
    :x_(0),
     y_(0),
     z_(0),
     size_(pSize),
     sizeOffset_(0),
     yOffset_(0),
     texture_(pText),
     selected_(false),
     drawn_(false),
     fileName_(pName)
{
}

GlWidget::GlWidget(QWidget *parent) :
    Glview(60,parent),
    selectionMode_(HandEvent::SINGLE),
    boxSize_(BOX_SIZE),
    gridSize_(0),
    spacing_(DEFAULT_SPACING),
    zoomOffset_(0),
    maxZoom_(BOX_SIZE),
    grabbing_(false),
    fileExplorer_(QDir::home()),
    currentAnim_(IDLE)
{
    leapListener_.setReceiver(this);
    controller_.addListener(leapListener_);
    head_.x = 0.0;
    head_.y = 0.0;
    head_.z = 5.0;
    palmPos_.x = 0.0f;
    palmPos_.y = 0.0f;
    palmPos_.z = 5.0f;
    setCursor(Qt::BlankCursor);
    reloadFolder();
}

GlWidget::~GlWidget()
{
    controller_.removeListener(leapListener_);
}

void GlWidget::initializeGL()
{
    loadTexture("../code/ressources/box.png", CRATE);
    loadTexture("../code/ressources/metal.jpg", METAL);
    loadTexture("../code/ressources/Folder.png", FOLDER);
    loadTexture("../code/ressources/music.png", MUSIC);
    loadTexture("../code/ressources/picture.png", PICTURE);
    loadTexture("../code/ressources/text.png", TEXT);
    loadTexture("../code/ressources/video.png", VIDEO);

    glEnable(GL_TEXTURE_2D);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void GlWidget::resizeGL(int width, int height)
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

void GlWidget::paintGL()
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

    //place the camera like the real head and look at the center
    gluLookAt(head_.x,head_.y,head_.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,0.0f);

    // Objects
    drawPalmPos();
    computeTube(10);
    handleSelection();
    drawCurrentGrid();
    handleGrab();
    displayPath();
}

//helper function, loads a texture and assign it to an enum value
//to help retrieve it later
void GlWidget::loadTexture(QString textureName, texId_t pId)
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
void GlWidget::drawCube(texId_t PtextureId, float pCenterX, float pCenterY,float pCenterZ, float pSize)
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
void GlWidget::drawTile(texId_t PtextureId, float pCenterX, float pCenterY,float pCenterZ, float pSize)
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

void GlWidget::drawTile(const item_t& pItem)
{
    drawTile(pItem.texture_,
             pItem.x_,
             pItem.y_ + pItem.yOffset_,
             pItem.z_,
             pItem.size_ + pItem.sizeOffset_);
    if (pItem.fileName_.size() > 0)
    {
        //text is masked by cubes,
        //draw under to see it clearly
        renderText(pItem.x_ - pItem.size_/2.0f,
                   pItem.y_+pItem.yOffset_ - pItem.size_,
                   pItem.z_ + pItem.size_/2.0f,
                   pItem.fileName_);
    }
}

//overloaded function for ease of use
void GlWidget::drawCube(const item_t& pCube)
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
void GlWidget::drawPalmPos()
{
    //normalize leap coordinates to our box size
   drawCube(METAL,
            palmPos_.x ,
            palmPos_.y,
            palmPos_.z, BOX_SIZE/(4*gridSize_));
}

//init the view with a certain amount of cubes
void GlWidget::generateCubes(texId_t pTexture, int pNbCubes)
{
    itemList_.clear();
    for (int i=0; i<pNbCubes; ++i)
    {
        item_t item("",spacing_/3.0f, pTexture);
        itemList_.append(item);
    }
}

/* Display a 2D grid and translate it according to the zoom
 * offset, when hand is on a line, make the line higher
 * to better see items
 */
void GlWidget::computeWaveGrid(int pItemPerLine)
{
    gridSize_ = pItemPerLine;
    spacing_ = boxSize_/gridSize_;
    float itemSize = spacing_/2;

    float offset = (gridSize_-1)*spacing_/2;

    QMutexLocker locker(&mutexList_);
    //maxZoom is the limit for the offset
    maxZoom_ = (itemList_.size()/gridSize_)*2*spacing_;

    QList<item_t>::iterator it;
    int row = 0, col = 0;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
    {
      it->size_ = itemSize;
      it->x_ = col*spacing_ - offset;
      it->y_ = -boxSize_/3.0;
      it->z_ = -row*2*spacing_ + zoomOffset_; //along negative z
      col += 1;
      if ( col >= pItemPerLine )
      {
          row++;
          col = 0;
      }
    }
    handleHovering();
}

//creates a vertical "tube" with items around its edges
void GlWidget::computeTube(int pItemPerCircle)
{
    //nb of items per circle in the cylinder
    gridSize_ = pItemPerCircle;

    //angle between each branch in radians
    float angle = 2*PI/pItemPerCircle;
    float radius = boxSize_/2;

    //distance between items in a circle inside the box
    spacing_ = (float)boxSize_*PI/(float)gridSize_;
    spacing_/=2.0f; // half the distance
    float itemSize = spacing_/2.0f;

    QMutexLocker locker(&mutexList_);

    //maxZoom is the limit for the offset
    maxZoom_ = (itemList_.size()/gridSize_)*2*spacing_;

    QList<item_t>::iterator it;
    int posAngle = 0, circleNb = 0;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
    {
      it->size_ = itemSize;
      it->x_ = cos(posAngle*angle)*radius;
      it->y_ = zoomOffset_ - 2*spacing_*circleNb - (posAngle*2*spacing_)/gridSize_;

      //the nearest item is at z = 0 (offset by boxSize/2)
      it->z_ = -radius + sin(posAngle*angle)*radius;
      posAngle += 1;
      if ( posAngle >= pItemPerCircle )
      {
          circleNb++;
          posAngle = 0;
      }
    }
}

void GlWidget::handleHovering()
{
    QList<item_t>::iterator it;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
    {
        if (abs(palmPos_.z - it->z_) <= spacing_/2.0f)
        {
            if (it->yOffset_ < boxSize_/3.0)
                it->yOffset_ += boxSize_/30.0;
        }
        else if (it->yOffset_ > 0.0f)
            it->yOffset_ -= boxSize_/30.0f;
    }
}

void GlWidget::handleGrab()
{
    if ( grabbing_ )
    {
        if ( lastPos_.empty() || lastPos_.size() < grabList_.size() )
        {
            lastPos_.clear();
            for (int i = 0; i < grabList_.size(); i++)
            {
                Vector pos;
                int item = grabList_.values()[i];
                pos.x = itemList_[item].x_;
                pos.y = itemList_[item].y_;
                pos.z = itemList_[item].z_;
                lastPos_.append(pos);
            }
        }

        int nbItem = grabList_.size();
        for ( int i = 0; i < nbItem; i++)
        {
            item_t item;
            const item_t& realItem = itemList_[grabList_.values()[i]];
            item.texture_ = realItem.texture_;
            if (i < lastPos_.size() )
            {
                Vector newPos = lastPos_[i];
                if (newPos.distanceTo(palmPos_) > spacing_/4.0f)
                {
                    newPos+=(palmPos_-newPos)/20.0f;
                }

                item.x_ = lastPos_[i].x ;
                item.y_ = lastPos_[i].y ;
                item.z_ = lastPos_[i].z ;
                lastPos_[i] = newPos;
                item.size_ = spacing_/4.0f;
            }
            drawTile(item);
        }
    }
    else
        lastPos_.clear();
}

/*generate a cubic grid from itemList_
 *the grid is always in a box of BOX_SIZE
 * so the more cubes, the more smaller it appears
 */
void GlWidget::computeGrid(float pBoxSize)
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
void GlWidget::drawCurrentGrid()
{
    QList<item_t>::iterator it;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
        drawTile(*it);
}

//find the closest cube from the palm center
int GlWidget::closestItem(float pTreshold)
{
    float minDist = 1000.0f;
    QList<item_t>::iterator it;
    int id = -1, i = 0;
    for (it = itemList_.begin(); it != itemList_.end(); it++)
    {
        Leap::Vector testV(it->x_,it->y_+it->yOffset_, it->z_);
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
void GlWidget::handleSelection()
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

void GlWidget::displayPath()
{
    QString path = fileExplorer_.absolutePath();
    QStringList folderList = path.split("/");
    QStringList::const_iterator it;
    int x = 50;
    int y = 20;

    renderText(x,y,"Current Folder");
    y+=20;
    for (it = folderList.cbegin(); it != folderList.cend(); it++)
    {
        renderText(x,y,*it);
        y += 20;
    }
}

void GlWidget::changeDirectory(const QString& pFolder)
{
    bool ok = false;
    if ( pFolder == "..")
        ok = fileExplorer_.cdUp();
    else
        ok = fileExplorer_.cd(pFolder);

    if ( ok )
        reloadFolder();
}

//generate the view items from the files in a folder
void GlWidget::reloadFolder()
{
    //protect access on the datalist
    timer_->stop();
    QMutexLocker locker(&mutexList_);
    grabList_.clear();
    grabbing_ = false;

    qDebug() << "loaded folder: "<< fileExplorer_.path();

    fileExplorer_.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    fileExplorer_.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

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
            QString ext = it->suffix().toLower();
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
            else if ( ext == "mp4" ||
                      ext == "avi" ||
                      ext == "mkv")
                texture = VIDEO;
        }
        //create new cube, size doesn't matter, recomputed each time
        item_t item(it->fileName(), 1.0f, texture);
        newList.append(item);
    }
    itemList_ = newList;
    zoomOffset_ = 0;
    timer_->start();
}

void GlWidget::customEvent(QEvent* pEvent)
{
    HandEvent* event = dynamic_cast<HandEvent*>(pEvent);
    if ( event )
    {
        //detect if hand is near an item
        int item = closestItem(spacing_);
        leapListener_.setItem(item);

        float offset = 0;
        //handle type of event
        switch (event->type() )
        {
        case HandEvent::Opened:
            //release nowhere, do nothing
            if ( grabbing_ && item == -1 )
            {
                grabbing_ = false;
                grabList_.clear();
            }
            break;
        case HandEvent::Closed:
            break;
        case HandEvent::Clicked:
            item = event->item();
            selectionMode_ = event->selectMode();
            slotSelect(item);
            break;
        case HandEvent::DoubleClicked:
            break;
        case HandEvent::Zoom:
            offset = event->zoom();
            if ( zoomOffset_ + offset <= maxZoom_ )
                zoomOffset_ += offset;
            if ( zoomOffset_ < 0 )
                zoomOffset_ = 0;
            break;
        case HandEvent::Swiped:
            changeDirectory("..");
            break;
        case HandEvent::Grabbed:
            if (event->item() != -1 && itemList_[event->item()].selected_ )
            {
                if (selectionMode_ == HandEvent::MULTIPLE )
                {
                    for (int i=0; i<itemList_.size(); i++)
                    {
                        if ( itemList_[i].selected_)
                            grabList_.insert(i);
                    }
                    grabbing_ = true;
                }
                else
                {
                    //only add new items to the set
                    grabList_.clear();
                    grabList_.insert(event->item());
                    grabbing_ = true;
                }
            }
            break;
       case HandEvent::Moved:
            //convert normalize hand pos to our interaction box
            palmPos_ = (event->pos()+Vector(-0.5f,-0.5f,-1.0f))*boxSize_*1.5f;
            break;
        default:
            break;
        }
    }
}

//update the camera position
void GlWidget::slotNewHead(head_t pPos)
{
    /*We inverse axes to compensate head position relative
     * to the cube.
     */
    head_.x = -pPos.x;
    head_.y = -pPos.y;
    head_.z =  pPos.z;
}

//move slightly the camera, via keyboard commands for example
void GlWidget::slotMoveHead(int pAxis, float pDelta)
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
void GlWidget::slotSelect(int pItem)
{
    //hand is on a single item
    if (pItem!= -1 )
    { 
        if ( selectionMode_ == HandEvent::SINGLE )
        {
            //select pItempreviously not selected
            if ( !itemList_[pItem].selected_ )
            {
                itemList_[pItem].selected_ = true;
                for( int i = 0; i < itemList_.size(); i++ )
                {
                    if (i != pItem)
                        itemList_[i].selected_ = false;
                }
            }
            else //open previously selected item
            {
                if ( pItem< fileExplorer_.entryInfoList().size() )
                {
                    QFileInfo info =  fileExplorer_.entryInfoList().at(pItem);
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
        else if (selectionMode_ == HandEvent::MULTIPLE )
        {
            itemList_[pItem].selected_ = !itemList_[pItem].selected_ ;
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
void GlWidget::slotPalmPos(Vector pPos)
{
    palmPos_ = pPos;
}
