#include "facetrack.h"
#include <iostream>
#include <iterator>
#include <cctype>

#define WEBCAM_WINDOW "webcam"

using namespace std;

Facetrack::Facetrack(string pCascadeFile)
    :capture_(0),
      x1_(0),y1_(0),x2_(0),y2_(0),
      cascadePath_(pCascadeFile),
      newFaceFound_(false),
      scale_(1.0)
{
    head_.x = 0;
    head_.z = 1.0;
    head_.y = 0;
}

Facetrack::~Facetrack()
{
    cvReleaseCapture(&capture_);
    cvDestroyWindow(WEBCAM_WINDOW);
}

void Facetrack::init(void)
{
    capture_ = cvCaptureFromCAM(CV_CAP_ANY);
    string path = DATADIR;
    path += cascadePath_;
    if (!cascade_.load(path))
    {
        throw string("Cascade file not found: ") + string(path);
    }
    if ( !capture_ )
    {
        throw string("Couldn't open webcam");
        namedWindow(WEBCAM_WINDOW);
    }
}

void Facetrack::showRaw(void)
{
    imshow(WEBCAM_WINDOW, rawFrame_);
}

QPixmap Facetrack::showFace(void)
{
    Point center;
    Scalar color =  CV_RGB(0,255,0);
    int radius;

    double aspect_ratio = (double)face_.width/face_.height;
    if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
    {
    center.x = cvRound((face_.x + face_.width*0.5)*scale_);
    center.y = cvRound((face_.y + face_.height*0.5)*scale_);
    radius = cvRound((face_.width + face_.height)*0.25*scale_);
    circle( frameCpy_, center, radius, color, 3, 8, 0 );
    }
    else
    rectangle( frameCpy_, cvPoint(cvRound(face_.x*scale_), cvRound(face_.y*scale_)),
    cvPoint(cvRound((face_.x + face_.width-1)*scale_), cvRound((face_.y + face_.height-1)*scale_)),
    color, 3, 8, 0);
    IplImage* img = new IplImage;
    img->imageData = (char*)frameCpy_.data;
    QPixmap pix;
    pix.fromImage(ipl2QImage(img));
    return pix;
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
    /*We use the haarcascade classifier
     * only take the first (biggest) face found
     */
    cascade_.detectMultiScale( gray, faces,
           1.1, 2, 0
           |CV_HAAR_FIND_BIGGEST_OBJECT
           |CV_HAAR_DO_ROUGH_SEARCH,
           Size(30, 30));

/* //for debug purposes
   vector<Rect>::const_iterator it;
    for( it = faces.begin(); it != faces.end(); it++)
    {
        cout<<"x: "<<it->x;
        cout<<", y: "<<it->y;
        cout<<", w: "<<it->width;
        cout<<", h: "<<it->height<<endl;
    }
*/
    //take coordinates of first face found
    if( faces.size() > 0 )
    {
        prevFace_ = face_;
        face_ = faces[0];
        newFaceFound_ = true;
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


//Parameters initially from wiimote configuration of
//Johny Chung Lee, adapted for the webcam here.

#define EYE_DISTANCE 10 // between 1 and 1000 depending on resolution
#define VERTICAL_ANGLE 0// allows to compensate if webcam is not parallel
                        // with the screen
#define WIIMOTE_ADJUST 0 // head height between -100 and 100
#define CAMERA_ABOVE true // camera above the screen generally

// Track head position with Johnny Chung Lee's trig stuff
// XXX: Note that positions should be float values from 0-1024
//      and 0-720 (width, height, respectively).
void Facetrack::WTLeeTrackPosition (float radPerPix)
{
    // Where is the middle of the head?
    float dx = (float)(x1_ - x2_), dy = (float)(y1_ - y2_);
    float pointDist = (float)sqrt(dx * dx + dy * dy);
    float angle = radPerPix * pointDist / 2.0;

    /* Set the head distance in units of screen size
     * creates more or less zoom
     */
    head_.z = ((float)EYE_DISTANCE / 1000.0) / (float)tan(angle);
    float aX = (x1_ + x2_) / 2.0f, aY = (y1_ + y2_) / 2.0f;

    // Set the head position horizontally
    head_.x = (float)sin(radPerPix * (aX - 512.0)) * head_.z;
    float relAng = (aY - 384.0) * radPerPix;

    // Set the head height
    head_.y = -0.5f + (float)sin((float)VERTICAL_ANGLE/ 100.0 + relAng) * head_.z;
    // And adjust it to suit our needs
    head_.y = head_.y + (float)WIIMOTE_ADJUST / 100.0;
    // And if our Wiimote is above our screen, adjust appropriately
    if (CAMERA_ABOVE)
        head_.y = head_.y + 0.5;

    cout<<"head: "<<head_.x<<" "<<head_.y<<" "<<head_.z<<endl;
}

QImage Facetrack::ipl2QImage(const IplImage *newImage)
{
    QImage qtemp;
    if (newImage && cvGetSize(newImage).width > 0)
    {
        int x;
        int y;
        char* data = newImage->imageData;

        qtemp= QImage(newImage->width, newImage->height,QImage::Format_RGB32 );
        for( y = 0; y < newImage->height; y++, data +=newImage->widthStep )
            for( x = 0; x < newImage->width; x++)
            {
                uint *p = (uint*)qtemp.scanLine (y) + x;
                *p = qRgb(data[x * newImage->nChannels+2],
                          data[x * newImage->nChannels+1],data[x * newImage->nChannels]);
            }
    }
    return qtemp;
}
