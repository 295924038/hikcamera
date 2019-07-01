#include "imagelabel.h"

#include <QString>
#include <QMessageBox>

#include "hikcamera.h"

QRect ImageLabel::getRect() const
{
    return QRect(_ptStart, _ptEnd) ;
}

bool ImageLabel::isValid() const
{
    return _bValid ;
}

void ImageLabel::mousePressEvent(QMouseEvent *ev)
{
    _bValid = false ;
    _ptStart = ev->pos() ;
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    _ptEnd = ev->pos() ;
    if ( _ptStart != _ptEnd )
        _bValid = true ;
}
