#include "glwidget.h"
#include <GL/glu.h>
#include <iostream>

glWidget::glWidget(QWidget *parent) :
    Glview(60,parent)
{
    head_.x = 0.0;
    head_.y = 0.0;
    head_.z = 5.0;
}

void glWidget::initializeGL()
{
    loadTexture("../code/ressources/box.png");

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
    // =======
   /* drawCube(CRATE,0,0,0,2.0f);
    glTranslatef(3,0,0);
    drawCube(CRATE,0,0,0,2.0f);
    */
    drawCube3DGrid(CRATE,0.5f,1.0f,5,5,5);

}

void glWidget::loadTexture(QString textureName)
{
    QImage qim_Texture;
    QImage qim_TempTexture;
    qim_TempTexture.load(textureName);
    qim_Texture = QGLWidget::convertToGLFormat( qim_TempTexture );
    glGenTextures( 1, &texture_[0] );
    glBindTexture( GL_TEXTURE_2D, texture_[0] );
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
