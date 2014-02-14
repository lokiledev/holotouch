#-------------------------------------------------
#
# Project created by QtCreator 2014-01-21T19:43:24
#
#-------------------------------------------------

QT       += core gui widgets opengl

TARGET = holotouch
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += /usr/local/include/opencv
#INCLUDEPATH += ../code/leapmotion

#opencv libraries
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lopencv_nonfree

#opengl lib
LIBS += -lGLU

#leapmotion lib
LIBS += -L"$$_PRO_FILE_PWD_/leapmotion"
LIBS += -lLeap

SOURCES += main.cpp \
    head_tracking/facetrack.cpp \
    mainwindow.cpp \
    glview.cpp \
    glwidget.cpp \
    leapmotion/LeapListener.cpp \
    leapmotion/HandEvent.cpp

HEADERS += \
    mainwindow.h \
    head_tracking/facetrack.h \
    glview.h \
    glwidget.h \
    tracking_defines.h \
    leapmotion/Leap.h \
    leapmotion/LeapListener.h \
    leapmotion/HandEvent.h
