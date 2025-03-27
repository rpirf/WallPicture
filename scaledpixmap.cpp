#include "scaledpixmap.h"
#include <QPainter>
#include <QGridLayout>

using namespace WP;

ScaledPixmap::ScaledPixmap(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ScaledPixmap::setPixmap(const QPixmap& pixmap)
{
    _pixmap = pixmap;
    update();
}

QPixmap ScaledPixmap::pixmap() const
{
    return _pixmap;
}

QSize ScaledPixmap::sizeHint() const
{
    return _pixmap.size();
}

void ScaledPixmap::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    if (false == _pixmap.isNull())
    {
        QSize widgetSize = size();
        QPixmap scaledPixmap = _pixmap.scaled(widgetSize, Qt::KeepAspectRatio);
        QPoint center((widgetSize.width() - scaledPixmap.width()) / 2,
            (widgetSize.height() - scaledPixmap.height()) / 2);
        painter.drawPixmap(center, scaledPixmap);
    }
    QWidget::paintEvent(event);
}
