#ifndef IMALELABEL_H
#define IMALELABEL_H

#include <atomic>

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QRect>

#include <opencv2/opencv.hpp>

/// 支持界面图片的点击消息处理
class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel ;

    QRect getRect() const ;

    bool isValid() const ;
protected:
    /// 覆盖父类版本
    void mousePressEvent( QMouseEvent *ev ) override ;

    /// 覆盖父类版本
    void mouseReleaseEvent( QMouseEvent *ev ) override ;
private:
    QPoint _ptStart = {0,0};
    QPoint _ptEnd = {0,0};
    std::atomic<bool> _bValid{false};
};

#endif // IMALELABEL_H
