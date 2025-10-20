#include "linkbutton.h"

#include <QFontMetrics>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

QSize LinkButton::sizeHint() const {
    QSize size;
    QFontMetrics fontMetrics(this->font());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    size.setWidth(fontMetrics.width(this->text()));
#else
    size.setWidth(fontMetrics.horizontalAdvance(this->text()));
#endif
    size.setHeight(fontMetrics.height());
    return size;
}
