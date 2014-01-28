#ifndef FACETRACK_CPP
#define FACETRACK_CPP

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QObject>

#define CASCADE "haarcascade_frontalface_alt2.xml"
#define DATADIR "../code/ressources/"

using namespace std;
using namespace cv;

class Facetrack : public QObject
{
    Q_OBJECT

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

public:
    Facetrack(string pCascadeFile = CASCADE);
    ~Facetrack();

    void init(void);
    void getNewImg(void);
    void showRaw(void);
    void showFace(void);
    void detectHead(void);
    void getCoordinates(void);

signals:
    void signalNewHeadPos(CvPoint* pNewPos);

};

#endif // FACETRACK_CPP
