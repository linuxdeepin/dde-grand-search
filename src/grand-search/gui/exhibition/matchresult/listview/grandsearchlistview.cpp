/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu <houchengqiu@uniontech.com>
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
#include "grandsearchlistview.h"
#include "grandsearchlistmodel.h"
#include "grandsearchlistdelegate.h"
#include "utils/utils.h"
#include "global/matcheditem.h"

#include <DGuiApplicationHelper>
#include <DStandardPaths>

#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QMouseEvent>

DCORE_USE_NAMESPACE
using namespace GrandSearch;

#define ICON_SIZE 24
#define ListItemTextMaxWidth      240      // 文本元素最大显示宽度

GrandSearchListview::GrandSearchListview(QWidget *parent)
    : DListView(parent)
{
    m_model = new GrandSearchListModel(0, 0, this);
    setModel(m_model);
    m_delegate = new GrandSearchListDelegate(this);
    setItemDelegate(m_delegate);
    setViewportMargins(0, 0, 0, 0);
    setUniformItemSizes(true);

    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setMouseTracking(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onItemClicked(const QModelIndex &)));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GrandSearchListview::onSetThemeType);

    m_themeType = DGuiApplicationHelper::instance()->themeType();
}

GrandSearchListview::~GrandSearchListview()
{

}

void GrandSearchListview::setMatchedItems(const MatchedItems &items)
{
    m_matchedItems = items;
    m_model->clear();
    for (MatchedItem item : m_matchedItems) {
        addRow(item);
    }
}

void GrandSearchListview::addRow(const MatchedItem &item)
{
    QStandardItem *newItem = new QStandardItem;
    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());

    QVariant searchMeta;
    searchMeta.setValue(item);
    m_model->setData(index, searchMeta, DATA_ROLE);

    // 添加悬浮提示
    QFontMetricsF fontWidth(DFontSizeManager::instance()->get(DFontSizeManager::T6));
    QString mtext = fontWidth.elidedText(item.name, Qt::ElideRight, ListItemTextMaxWidth);
    if (mtext != item.name) {
        m_model->setData(index, item.name, Qt::ToolTipRole);
    }

    // 设置icon
    QVariant iconVariantData;
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        const int size = static_cast<int>(ICON_SIZE * qApp->devicePixelRatio());
        const QSize iconSize(size, size);

        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                iconVariantData.setValue(QIcon(strIcon).pixmap(iconSize));
            else
                iconVariantData.setValue(Utils::defaultIcon(item).pixmap(iconSize));
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                iconVariantData.setValue(Utils::defaultIcon(item).pixmap(iconSize));
            else
                iconVariantData.setValue(icon.pixmap(iconSize));
        }
    }

    m_model->setData(index,iconVariantData, Qt::DecorationRole);
}

void GrandSearchListview::addRows(const MatchedItems &items)
{
    for (auto item : items)
        addRow(item);
}

int GrandSearchListview::rowCount()
{
    return m_model->rowCount();
}

int GrandSearchListview::getThemeType() const
{
    return m_themeType;
}

void GrandSearchListview::clear()
{
    if (m_model)
        m_model->clear();
}

void GrandSearchListview::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();
    Utils::openMatchedItem(item);
    emit sigItemClicked();

    qDebug() << QString("GrandSearchListView::onItemClicked item clicked r[%1] c[%2]").arg(index.row()).arg(index.column());
}

void GrandSearchListview::onSetThemeType(int type)
{
    m_themeType = type;
}

void GrandSearchListview::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        if (this->currentIndex() != index) {
            setCurrentIndex(index);

            // 通知搜索输入框更新应用图标
            MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();
            emit sigAppIconChanged(Utils::appIconName(item));

            // 通知其它列表取消选中
            emit sigSelectItemByMouse(this);

            return;
        }
    }

    return DListView::mouseMoveEvent(event);
}

bool GrandSearchListview::event(QEvent *event)
{
    Q_UNUSED(event)
    return QListView::event(event);
}

bool GrandSearchListview::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverMove :
    case QEvent::HoverEnter :
        // 禁用悬浮事件，否则需要对悬浮位置的背景做特殊绘制（默认会比选中的颜色偏亮）
        return true;
    default:break;
    }
    return DListView::viewportEvent(event);
}

QString GrandSearchListview::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}
