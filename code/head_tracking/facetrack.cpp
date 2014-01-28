#include "facetrack.hpp"
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

void Facetrack::showFace(void)
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
