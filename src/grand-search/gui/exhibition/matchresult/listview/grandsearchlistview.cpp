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

GrandSearchListView::GrandSearchListView(QWidget *parent)
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
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GrandSearchListView::onSetThemeType);

    m_themeType = DGuiApplicationHelper::instance()->themeType();
}

GrandSearchListView::~GrandSearchListView()
{

}

void GrandSearchListView::setMatchedItems(const MatchedItems &items, const QString &group)
{
    m_matchedItems = items;
    m_model->clear();
    for (MatchedItem item : m_matchedItems) {
        addRow(item, group);
    }
}

void GrandSearchListView::addRow(const MatchedItem &item, const QString &group)
{
    QStandardItem *newItem = new QStandardItem;
    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());

    setData(index, item, group);
}

void GrandSearchListView::addRows(const MatchedItems &items, const QString &group)
{
    for (auto item : items)
        addRow(item, group);
}

int GrandSearchListView::insertRow(int nRow, const MatchedItem &item, const QString &group)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRow(item, group);
        return m_model->rowCount() - 1;
    }

    QStandardItem *newItem = new QStandardItem;
    m_model->insertRow(nRow, newItem);

    QModelIndex index = m_model->index(nRow, 0, QModelIndex());
    setData(index, item, group);

    return nRow;
}

void GrandSearchListView::insertRows(int nRow, const MatchedItems &items, const QString &group)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRows(items, group);
        return;
    }

    for (int i = 0; i < items.size(); i++) {
        insertRow(nRow,items[i], group);
    }
}

void GrandSearchListView::removeRows(int nRow, int nCount)
{
    m_model->removeRows(nRow, nCount);
}

MatchedItems GrandSearchListView::groupItems(const QString &group)
{
    MatchedItems items;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString tempGroup = m_model->data(m_model->index(i, 0, QModelIndex()), GROUP_ROLE).toString();
        if (tempGroup == group)
            items.push_back(m_model->data(m_model->index(i, 0, QModelIndex()), DATA_ROLE).value<MatchedItem>());
    }

    return items;
}

int GrandSearchListView::lastShowRow(const QString &group)
{
    if (group.isEmpty())
        return -1;

    int nRow = -1;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString tempGroup = m_model->data(m_model->index(i, 0, QModelIndex()), GROUP_ROLE).toString();
        if (tempGroup == group)
            nRow = i;
        else if (nRow != -1)
            break;
    }

    return nRow;
}

int GrandSearchListView::rowCount()
{
    return m_model->rowCount();
}

int GrandSearchListView::getThemeType() const
{
    return m_themeType;
}

void GrandSearchListView::clear()
{
    if (m_model)
        m_model->clear();
}

void GrandSearchListView::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (this->currentIndex() != index) {
        setCurrentIndex(index);

        MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();

        // 通知匹配结果界面选择有改变
        emit sigSelectItemByMouse(item);

        return;
    }
    else {
        MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();
        Utils::openMatchedItem(item);
        emit sigItemClicked();
    }
}

void GrandSearchListView::onSetThemeType(int type)
{
    m_themeType = type;
}

bool GrandSearchListView::event(QEvent *event)
{
    Q_UNUSED(event)
    return QListView::event(event);
}

QString GrandSearchListView::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}

void GrandSearchListView::setData(const QModelIndex& index, const MatchedItem &item, const QString &group)
{
    if (!index.isValid())
        return;

    QVariant searchMeta;
    searchMeta.setValue(item);
    m_model->setData(index, searchMeta, DATA_ROLE);
    m_model->setData(index, QVariant(group), GROUP_ROLE);

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
        const int size = static_cast<int>(ICON_SIZE);
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
