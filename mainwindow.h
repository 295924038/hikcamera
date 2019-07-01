#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hikcamera.h"
#include <boost/shared_ptr.hpp>
#include "tools/singleton.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openCamera_clicked();

    void on_cb_autoTakePictures_stateChanged(int arg1);

    void on_takePicture_clicked();

    void showPicture();

private:
    Ui::MainWindow *ui;
    Singleton<camera> _camera ;
    std::shared_ptr<camera> _camera_ptr ;
    bool bContinue;
};

#endif // MAINWINDOW_H
