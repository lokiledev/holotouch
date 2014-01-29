#ifndef FACETRACK_CPP
#define FACETRACK_CPP

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QObject>
#include <QImage>
#include <QPixmap>

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
    typedef struct {
        float x;
        float y;
        float z;
    }head_t;
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
    QPixmap showFace(void);
    void detectHead(void);
    void getCoordinates(void);
    void WTLeeTrackPosition (float radPerPix);
    QImage ipl2QImage(const IplImage *newImage);

signals:
    void signalNewHeadPos(CvPoint* pNewPos);

};

#endif // FACETRACK_CPP
