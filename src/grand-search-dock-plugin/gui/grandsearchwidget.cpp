/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "grandsearchwidget.h"
#include "contacts/interface/grandsearchinterface.h"

#include "constants.h"
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#include <QPainter>
#include <QDebug>

GrandSearchWidget::GrandSearchWidget(QWidget *parent)
    : QWidget(parent)
{
    m_grandSearchInterface = new GrandSearchInterface(this);
    connect(m_grandSearchInterface, &GrandSearchInterface::VisibleChanged, this, &GrandSearchWidget::grandSearchVisibleChanged);

    bool isRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.dde.GrandSearch");
    if (isRegistered) {
        m_grandSearchVisible = m_grandSearchInterface->IsVisible();
    } else {
        m_grandSearchVisible = false;
    }

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GrandSearchWidget::updateIcon);
}

GrandSearchWidget::~GrandSearchWidget()
{

}

void GrandSearchWidget::updateIcon()
{
    QString iconName = "grand-search";

    DGuiApplicationHelper::ColorType type = DGuiApplicationHelper::instance()->themeType();
    if (DGuiApplicationHelper::LightType == type) {
        iconName += QStringLiteral("-dark");
    } else {
        // DGuiApplicationHelper::DarkType
        iconName += QStringLiteral("-light");
    }
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/icons/%1.svg").arg(iconName)));

    const int size = PLUGIN_ICON_MAX_SIZE;
    const qreal ratio = devicePixelRatioF();

    m_pixmap = m_icon.pixmap(int(size * ratio), int(size * ratio));
    m_pixmap.setDevicePixelRatio(ratio);

    update();
}

QString GrandSearchWidget::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey)

    m_grandSearchVisible = !m_grandSearchVisible;
    m_grandSearchInterface->SetVisible(m_grandSearchVisible);

    return QString();
}

void GrandSearchWidget::grandSearchVisibleChanged(bool vidible)
{
    m_grandSearchVisible = vidible;
}

void GrandSearchWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateIcon();
}

void GrandSearchWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    const QRectF &rf = QRectF(rect());
    const QRectF &rfp = QRectF(m_pixmap.rect());
    painter.drawPixmap(rf.center() - rfp.center() / m_pixmap.devicePixelRatioF(), m_pixmap);
}
