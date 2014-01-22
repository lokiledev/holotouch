typedef struct {
    float x;
    float y;
    float z;
}head_t;
int headtrack(int *x1, int *y1, int *x2, int *y2, int lissage, int smooth, int delay, int opt_scale);
void endThread(void);
void WTLeeTrackPosition (head_t* head,
                         float x1,
                         float y1,
                         float x2,
                         float y2,
                         float radPerPix);


#define DEPTH_ADJUST 100 // between 1 and 1000, zoom effect
#define SCALE 50 // scale of head between 10 and 100
