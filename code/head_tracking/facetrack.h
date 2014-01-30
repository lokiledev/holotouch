#ifndef FACETRACK_CPP
#define FACETRACK_CPP

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QObject>
#include <QPixmap>

#include "tracking_defines.h"

#define CASCADE "haarcascade_frontalface_alt2.xml"
#define DATADIR "../code/ressources/"

#define DEPTH_ADJUST 100 // between 1 and 1000, zoom effect
#define SCALE 40 // scale of head between 10 and 100

using namespace std;
using namespace cv;

class Facetrack : public QObject
{
    Q_OBJECT
public:

private:
    // Link to capture device ie Webcam.
    CvCapture* capture_;

    /* coordinates in pixels in the image of the bottom left and top right
     * rectangle around the face.
     */
    int x1_,y1_,x2_,y2_;

    // handlers for image data
    Mat rawFrame_;
    Mat frameCpy_;

    string cascadePath_;
    CascadeClassifier cascade_;
    Rect face_;
    Rect prevFace_;
    bool newFaceFound_;
    double scale_;

    head_t head_;

public:
    Facetrack(string pCascadeFile = CASCADE);
    ~Facetrack();

    void init(void);
    void getNewImg(void);
    void showRaw(void);
    void drawFace(void);
    void showFace(void);
    void detectHead(void);
    QPixmap getPixmap(void);
    void getCoordinates(void);
    void WTLeeTrackPosition (float radPerPix);
    QImage putImage(const Mat& mat);

signals:
    void signalNewHeadPos(head_t pNewPos);

};

#endif // FACETRACK_CPP
