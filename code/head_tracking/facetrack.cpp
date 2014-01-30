#include "facetrack.h"
#include <iostream>
#include <iterator>
#include <cctype>
#include <math.h>

#define WEBCAM_WINDOW "webcam"

using namespace std;

Facetrack::Facetrack(string pCascadeFile)
    :capture_(0),
      x1_(0),y1_(0),x2_(0),y2_(0),
      cascadePath_(pCascadeFile),
      newFaceFound_(false),
      scale_(MOVE_SCALE),
      fov_(WEBCAM_FOV)
{
    head_.x = 0;
    head_.z = 5.0;
    head_.y = 0;
}

Facetrack::~Facetrack()
{
    cvReleaseCapture(&capture_);
}

void Facetrack::init(void)
{
    capture_ = cvCaptureFromCAM(CV_CAP_ANY);

    if ( !capture_ )
    {
        throw string("Couldn't open webcam, device busy.\nTry closing other webcam apps or reboot");
    }

    string path = DATADIR;
    path += cascadePath_;
    if (!cascade_.load(path))
    {
        throw string("Cascade file not found: ") + string(path);
    }

}

void Facetrack::showRaw(void)
{
    imshow(WEBCAM_WINDOW, rawFrame_);
}

void Facetrack::drawFace(void)
{
    Point center;
    Scalar color =  CV_RGB(0,255,0);
    int radius;

    double aspect_ratio = (double)face_.width/face_.height;
    if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
    {
    center.x = cvRound((face_.x + face_.width*0.5));
    center.y = cvRound((face_.y + face_.height*0.5));
    radius = cvRound((face_.width + face_.height)*0.25);
    circle( frameCpy_, center, radius, color, 3, 8, 0 );
    }
    else
    rectangle(frameCpy_, cvPoint(cvRound(face_.x), cvRound(face_.y)),
              cvPoint(cvRound(face_.x + face_.width-1),
                      cvRound(face_.y + face_.height-1)),
              color, 3, 8, 0);
}

QPixmap Facetrack::getPixmap(void)
{
    return QPixmap::fromImage(putImage(frameCpy_));
}

void Facetrack::showFace(void)
{
    imshow(WEBCAM_WINDOW, frameCpy_);
}

void Facetrack::getNewImg(void)
{
    IplImage* iplImg = cvQueryFrame(capture_);
    rawFrame_ = iplImg;
    if( !rawFrame_.empty() )
    {
        if( iplImg->origin == IPL_ORIGIN_TL )
            rawFrame_.copyTo( frameCpy_ );
        else
            flip( rawFrame_, frameCpy_, 0 );
    }
}

void Facetrack::detectHead(void)
{
    Mat gray;
    cvtColor( frameCpy_, gray, CV_BGR2GRAY );
    vector<Rect> faces;
    newFaceFound_ = false;
    /*We use the haarcascade classifier
     * only take the first (biggest) face found
     */
    cascade_.detectMultiScale( gray, faces,
           1.1, 2, 0
           |CV_HAAR_FIND_BIGGEST_OBJECT
           |CV_HAAR_DO_ROUGH_SEARCH,
           Size(10, 10));

    //take coordinates of first face found
    if( faces.size() > 0 )
    {
        stabilize(faces[0]);
        getCoordinates();
    }
 }

void Facetrack::getCoordinates(void)
{
    int x1,y1,x2,y2;
    x1=y1=x2=y2=0;
    x1 = face_.x;
    y1 = face_.y;
    x2 = x1 + face_.width;
    y2 = y1 + face_.height;

    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;
}

/* Convert the rectangle found in 2D to 3D pos in unit box
 */
// Track head position with Johnny Chung Lee's trig stuff
// XXX: Note that positions should be float values from 0-1024
//      and 0-720 (width, height, respectively).
void Facetrack::WTLeeTrackPosition (void)
{
    /*Find nb of rad/pixel from webcam resolution
     * and webcam field of view (supposed 45° by default)
    */
    int fovWidth = frameCpy_.cols;
    float camW2 = (float)frameCpy_.cols/2;
    float camH2 = (float)frameCpy_.rows/2;
    float radPerPix = (fov_/fovWidth);

    //get the size of the head in degrees (relative to the field of view)
    float dx = (float)(x1_ - x2_), dy = (float)(y1_ - y2_);
    float pointDist = (float)sqrt(dx * dx + dy * dy);
    float angle = radPerPix * pointDist / 2.0;

    /* Set the head distance in units of screen size
     * creates more or less zoom
     */
    head_.z = (float)(DEPTH_ADJUST*((AVG_HEAD_MM / 2) / std::tan(angle)) / (float)SCREENHEIGHT);

    //average distance = center of the head
    float aX = (x1_ + x2_) / 2.0f, aY = (y1_ + y2_) / 2.0f;

    // Set the head position horizontally
    head_.x = scale_*((float)sin(radPerPix * (aX - camW2)) * head_.z);
    float relAng = (aY - camH2) * radPerPix;

    // Set the head height
    head_.y = scale_*(-0.5f + (float)sin((float)VERTICAL_ANGLE/ 100.0 + relAng) * head_.z);

    // we suppose in general webcam is above the screen like in most laptops
    if (CAMERA_ABOVE)
        head_.y = head_.y + 0.5f + (float)sin(relAng)*head_.z;

    cout<<"head: "<<head_.x<<" "<<head_.y<<" "<<head_.z<<endl;
    emit signalNewHeadPos(head_);
}

QImage Facetrack::putImage(const Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else
    {
        throw string("ERROR: Mat could not be converted to QImage.");
    }
}

/*
 * Smooth the movement
 * TODO
 */
void Facetrack::stabilize(Rect pNewFace)
{
    prevFace_ = face_;
    face_ = pNewFace;
    newFaceFound_ = true;
    getCoordinates();
    WTLeeTrackPosition();
}

bool Facetrack::isNewFace(void)
{
    return newFaceFound_;
}
