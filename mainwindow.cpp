#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include "base.h"
#include <thread>
#include <iostream>
#include <QTimer>
#include "camera_image.h"

using namespace std;
using namespace cv;

static QTimer qTimer ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _camera_ptr(Singleton<camera>::get()),
    bContinue{false},
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openCamera_clicked()
{
    try {

        static QTimer qTimer ;

        if ( bContinue ) {
            qTimer.stop();
            QObject::disconnect(&qTimer, SIGNAL(timeout()),this, SLOT(showPicture())) ;
            ui->openCamera->setText("open");
            ui->status_label->setText("相机状态：关闭");
        }
        else {
            QObject::connect(&qTimer, SIGNAL(timeout()),this, SLOT(showPicture())) ;
            qTimer.start(100);
            ui->openCamera->setText("close");
            ui->status_label->setText("相机状态：开启");
        }
        bContinue = !bContinue ;
    }
    catch ( boost::exception const &e ) {
        ui->status_label->setText("相机状态：出错");
    }
}


void MainWindow::showPicture()
{
    cv::Mat mat = _camera_ptr->getImg() ;
    if ( mat.data ) {
        // 可获取最新标定的图片及对应的区域
        auto rect = ui->picture->getRect();
        // mat 为最新的图片
        static cv::Ptr<cv::Tracker> track ;
        static cv::Rect2d roi ;
        static bool bLastValid = false ;
        static size_t count = 0 ;
        cv::Mat rgb = mat.clone();
        bool bRGB = false ;
        double px = (mat.size().width) / (ui->picture->width());
        double py = (mat.size().height) / (ui->picture->height());
        static bool bRoiValid = false ;
        if ( !bLastValid && ui->picture->isValid() ) {
            if ( QRect{0, 0, 1, 1} != rect ) {
                roi = cv::Rect2d( rect.left() * px, rect.top() * py, px*(rect.right()-rect.left()), py*(rect.bottom()-rect.top()) ) ;
                if ( greaterd( roi.width, 0 ) && greaterd(roi.height, 0) ) {
                    track = cv::Tracker::create("MIL");
                    track->init(mat, roi) ;
                    rgb = mat;
                    bRoiValid = true ;
                }
                else {
                    bRoiValid = false ;
                }
            }
            bLastValid = true ;
        }
        else if ( ui->picture->isValid() && bRoiValid){
            track->update(mat, roi) ;
            cv::cvtColor(mat, rgb, CV_GRAY2RGB) ;
            cv::rectangle(rgb, roi, cv::Scalar(0,255,0), 2,1);
            bRGB = true ;
        }
        if ( !ui->picture->isValid() ) {
            bLastValid = false ;
        }
        auto fmt = QImage::Format_RGB888 ;
        if ( !bRGB ) {
            fmt = QImage::Format_Indexed8 ;
        }

        QImage image((uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, fmt) ;
        QPixmap pixmap = QPixmap::fromImage(image);
        ui->picture->setPixmap(pixmap);
        time_t tm = time( nullptr ) ;
        static time_t tmLast = tm ;
        ++count ;
        if ( tmLast != tm ) {
            tmLast = tm ;
            count = 0 ;
        }
    }
}

void MainWindow::on_cb_autoTakePictures_stateChanged(int arg1)
{
    static bool bAutoSaveImg = false ;
    if ( 0 == arg1 ) {
        bAutoSaveImg = false ;
    }
    else if ( 2 == arg1 ) {
        bAutoSaveImg = true ;
    }
    if ( bAutoSaveImg ) {
        std::thread([this, &bAutoSaveImg](){
            int i = 0 ;
            while ( bAutoSaveImg ) {
                auto mat = _camera_ptr->takePicture() ;
                saveImg(mat, "", std::to_string(i++) + ".jpg") ;
            }
            std::cout << "停止自动拍照" << std::flush ;
        }).detach() ;
    }
}

void MainWindow::on_takePicture_clicked()
{
    saveImg(_camera_ptr->takePicture());
}

void MainWindow::on_save_clicked()
{
    auto widthValue = ui->widthValue->text().toInt();
    auto heightValue = ui->heightValue->text().toInt();
    auto offsetX = ui->offsetXValue->text().toInt();
    auto offsetY = ui->offsetYValue->text().toInt();

    auto exposureTime = ui->exposureValue->text().toInt();
    auto gain = ui->gainValue->text().toInt();

    _camera_ptr->setRoi(widthValue,heightValue,offsetX,offsetY);

    if(exposureTime > 0)
    {
        _camera_ptr->setExposure(exposureTime);
    }
    if(gain > 0)
    {
        _camera_ptr->setGain(gain);
    }
}
