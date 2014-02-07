#include "facetrack.h"
#include <iostream>
#include <iterator>
#include <cctype>
#include <math.h>

#define WEBCAM_WINDOW "webcam"

using namespace std;


Facetrack::coord_t::coord_t(int pX1, int pY1, int pX2, int pY2)
{
    x1 = pX1;
    y1 = pY1;
    x2 = pX2;
    y2 = pY2;
}

Facetrack::coord_t::coord_t(Rect pRect)
{
    x1 = pRect.x;
    y1 = pRect.y;
    x2 = pRect.x + pRect.width;
    y2 = pRect.y + pRect.height;
}

Rect Facetrack::coord_t::toRect(void)
{
    Rect res;
    res.x = x1;
    res.y = y1;
    res.width = x2-x1;
    res.height = y2-y1;
    return res;
}

/*only one for all the instances
 * all ones = simple average
 */
const int Facetrack::weights_[NB_SAMPLE_FILTER]= {1,1,1,1,1,1,1,1,1,1};

Facetrack::Facetrack(string pCascadeFile)
    :capture_(0),
      cascadePath_(pCascadeFile),
      currentFace_(0,0,0,0),
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
    capture_ = cvCaptureFromCAM(0);

    // try to open 2 different cams, else fail.
    // 0 = embedded cam like in laptops,
    // 1 = usb cam
    if ( !capture_ )
    {
        capture_ = cvCaptureFromCAM(1);
        if ( !capture_ )
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
    Rect face = currentFace_.toRect();
    double aspect_ratio = (double)face.width/face.height;
    if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
    {
    center.x = cvRound((face.x + face.width*0.5));
    center.y = cvRound((face.y + face.height*0.5));
    radius = cvRound((face.width + face.height)*0.25);
    circle( frameCpy_, center, radius, color, 3, 8, 0 );
    }
    else
    rectangle(frameCpy_, cvPoint(cvRound(face.x), cvRound(face.y)),
              cvPoint(cvRound(face.x + face.width-1),
                      cvRound(face.y + face.height-1)),
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
    }
    else
    {
        //recenter the camera
        Rect center;
        center.x = frameCpy_.cols/2 - 50;
        center.y = frameCpy_.rows/2 - 50;
        center.width = 100;
        center.height = 100;
        stabilize(center);
    }
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
    float dx = (float)(currentFace_.x1 - currentFace_.x2);
    float dy = (float)(currentFace_.y1 - currentFace_.y2);
    float pointDist = (float)sqrt(dx * dx + dy * dy);
    float angle = radPerPix * pointDist / 2.0;

    /* Set the head distance in units of screen size
     * creates more or less zoom
     */
    head_.z = (float)(DEPTH_ADJUST + scale_*((AVG_HEAD_MM / 2) / std::tan(angle)) / (float)SCREENHEIGHT);

    //average distance = center of the head
    float aX = (currentFace_.x1 + currentFace_.x2) / 2.0f;
    float aY = (currentFace_.y1 + currentFace_.y2) / 2.0f;

    // Set the head position horizontally
    head_.x = scale_*((float)sin(radPerPix * (aX - camW2)) * head_.z);
    float relAng = (aY - camH2) * radPerPix;

    // Set the head height
    head_.y = scale_*(-0.5f + (float)sin((float)VERTICAL_ANGLE/ 100.0 + relAng) * head_.z);

    // we suppose in general webcam is above the screen like in most laptops
    if (CAMERA_ABOVE)
        head_.y = head_.y + 0.5f + (float)sin(relAng)*head_.z;

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
    coord_t newFace(pNewFace);
    prevFaces_.push_back(newFace);
    if (prevFaces_.size() > NB_SAMPLE_FILTER)
        prevFaces_.pop_front();

    coord_t result;
    int cumul = 0;

    //compute the weighted average
    for(int i=0; i < prevFaces_.size(); ++i)
    {
        cumul += weights_[i];
        result.x1 += prevFaces_.at(i).x1*weights_[i];
        result.y1 += prevFaces_.at(i).y1*weights_[i];
        result.x2 += prevFaces_.at(i).x2*weights_[i];
        result.y2 += prevFaces_.at(i).y2*weights_[i];
    }

    //don't forget to normalize
    result.x1 /=  cumul;
    result.y1 /= cumul;
    result.x2 /= cumul;
    result.y2 /= cumul;

    currentFace_ = result;
    newFaceFound_ = true;
    WTLeeTrackPosition();
}

bool Facetrack::isNewFace(void)
{
    return newFaceFound_;
}
