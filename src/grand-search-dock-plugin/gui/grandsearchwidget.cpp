// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchwidget.h"

#include "interfaces/grandsearchinterface.h"

#include "constants.h"
#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DStyleHelper>

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

GrandSearchWidget::GrandSearchWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    QString iconName("grand-search-light");
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/icons/%1.svg").arg(iconName)));

    m_grandSearchInterface = new GrandSearchInterface(this);
    connect(m_grandSearchInterface, &GrandSearchInterface::VisibleChanged, this, &GrandSearchWidget::grandSearchVisibleChanged);

    bool isRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.dde.GrandSearch");
    if (isRegistered) {
        m_grandSearchVisible = m_grandSearchInterface->IsVisible();
    } else {
        m_grandSearchVisible = false;
    }
}

GrandSearchWidget::~GrandSearchWidget()
{

}

QString GrandSearchWidget::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey)

    m_grandSearchVisible = !m_grandSearchVisible;
    m_grandSearchInterface->SetVisible(m_grandSearchVisible);

    return QString();
}

void GrandSearchWidget::grandSearchVisibleChanged(bool visible)
{
    m_grandSearchVisible = visible;
}

void GrandSearchWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    /*!
     * 1.插件如果安装在插件区域即plugins目录下，需要插件自己绘制背景色，并根据鼠标移动、悬浮、点击的状态设置不同的透明度。
     *   否则图标将无背景，鼠标移动、悬浮、点击等无背景变色等效果。
     * 2.插件如果安装在托盘区域即plugins/system-tray目录下，插件只需要绘制自己的图标icon，不需要处理鼠标状态，不能自己绘制背景。
     *   否则将出现背景被重叠绘制，出现背景颜色非常深的问题。
     */
    QPixmap pixmap;
    QString iconName = "grand-search-light";
    int iconSize = PLUGIN_ICON_MAX_SIZE;

    QPainter painter(this);

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {

        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0.5);

            if (m_hover) {
                painter.setOpacity(0.6);
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0.1);

            if (m_hover) {
                painter.setOpacity(0.2);
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);

        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

        QPainterPath path;

        int minSize = std::min(width(), height());
        QRect rc(0, 0, minSize, minSize);
        rc.moveTo(rect().center() - rc.center());

        path.addRoundedRect(rc, radius, radius);
        painter.fillPath(path, color);
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        // 最小尺寸时，不画背景，采用深色图标
        iconName.replace(QString("-light"), PLUGIN_MIN_ICON_NAME);
    }

    painter.setOpacity(1);

    pixmap = loadSvg(iconName, QSize(iconSize, iconSize));

    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(pixmap.rect());
    painter.drawPixmap(rf.center() - rfp.center() / pixmap.devicePixelRatioF(), pixmap);
}

void GrandSearchWidget::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    update();

    QWidget::mousePressEvent(event);
}

void GrandSearchWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    m_hover = false;
    update();

    QWidget::mouseReleaseEvent(event);
}

void GrandSearchWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_hover = true;
    QWidget::mouseMoveEvent(event);
}

void GrandSearchWidget::leaveEvent(QEvent *event)
{
    m_hover = false;
    m_pressed = false;
    update();

    QWidget::leaveEvent(event);
}

const QPixmap GrandSearchWidget::loadSvg(const QString &fileName, const QSize &size) const
{
    const auto ratio = devicePixelRatioF();

    QPixmap pixmap;
    pixmap = QIcon::fromTheme(fileName, m_icon).pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);

    return pixmap;
}
