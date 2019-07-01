#-------------------------------------------------
#
# Project created by QtCreator 2019-06-24T22:51:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hikVision
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    hikcamera.cpp \
    imagelabel.cpp

HEADERS += \
        mainwindow.h \
    hikcamera.h \
    imagelabel.h

FORMS += \
        mainwindow.ui

PROROOT = /home/td/robotics/weldroidProject
PROJ_HOME=/home/td/robotics
OPENCV_HOME = /home/developer/opencv320
BOOST_HOME = /home/developer/boost1640

unix:!macx: LIBS += -L$$PWD/../../../../opt/MVS/lib/64/ -lMvCameraControl
INCLUDEPATH += $$PWD/../../../../opt/MVS/include
DEPENDPATH += $$PWD/../../../../opt/MVS/include


INCLUDEPATH +=  $${OPENCV_HOME}/include \
                $${BOOST_HOME}/include  \
                $${PROROOT}/include \

LIBS += -L$${OPENCV_HOME}/lib \
        -L$${BOOST_HOME}/lib/debug \
        -L$${PROROOT}/lib \
        -lpubfunc -ldevice -lcamera_image -limage -l3dvision -lbasic \
        -lgeometry  -lalgorithm -ltools -lbase \
        -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_tracking \
        -latomic \
        -lm3api \
        -lboost_filesystem -lboost_system -lboost_date_time \
        -lpcan \
        -lnlopt \
