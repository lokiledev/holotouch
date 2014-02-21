#ifndef FACETRACK_H
#define FACETRACK_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QObject>
#include <QPixmap>
#include <QQueue>
#include "tracking_defines.h"

#define CASCADE "haarcascade_frontalface_alt2.xml"
#define DATADIR "../code/ressources/"

using namespace std;
using namespace cv;

class Facetrack : public QObject
{
    Q_OBJECT
public:
    struct coord_t
    {
        int x1;
        int y1;
        int x2;
        int y2;
        coord_t(int pX1 = 0, int pY1 = 0, int pX2 = 0, int pY2 = 0);
        coord_t(Rect pRect);
        Rect toRect(void);
    };

private:
    // Link to capture device ie Webcam.
    CvCapture* capture_;

    /* coordinates in pixels in the image of the bottom left and top right
     * rectangle around the face.
     */


    // handlers for image data
    Mat rawFrame_;
    Mat frameCpy_;

    string cascadePath_;
    CascadeClassifier cascade_;

    //store last positions to filter them
    QQueue<coord_t> prevFaces_;
    coord_t currentFace_;

    //filters for faces coordinates
    static const int weights_[NB_SAMPLE_FILTER];
    bool newFaceFound_;
    double scale_;
    double fov_;

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
    void WTLeeTrackPosition ();
    QImage putImage(const Mat& mat);
    void stabilize(Rect pNewFace);
    bool isNewFace(void);

signals:
    void signalNewHeadPos(head_t pNewPos);

};

#endif // FACETRACK_H
