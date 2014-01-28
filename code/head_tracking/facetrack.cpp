#include "facetrack.hpp"
#include <iostream>
#include <iterator>
#include <cctype>

#define WEBCAM_WINDOW "webcam"

using namespace std;

Facetrack::Facetrack(string pCascadeFile)
    :capture_(0),
      x1_(0),y1_(0),x2_(0),y2_(0),
      cascadePath_(pCascadeFile)
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

void Facetrack::showImg(void)
{
    imshow(WEBCAM_WINDOW, rawFrame_);
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
    cascade_.detectMultiScale( gray, faces,
           1.1, 2, 0
           |CV_HAAR_FIND_BIGGEST_OBJECT
           |CV_HAAR_DO_ROUGH_SEARCH,
           Size(30, 30));

   vector<Rect>::const_iterator it;
    for( it = faces.begin(); it != faces.end(); it++)
    {
        cout<<"x: "<<it->x;
        cout<<", y: "<<it->y;
        cout<<", w: "<<it->width;
        cout<<", h: "<<it->height<<endl;
    }
}
