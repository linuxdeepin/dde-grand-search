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

#include <DGuiApplicationHelper>
#include <DStandardPaths>

#include <QDebug>
#include <QFileInfo>
#include <QPainter>

DCORE_USE_NAMESPACE

GrandSearchListview::GrandSearchListview(QWidget *parent)
    : DListView(parent)
{
    m_model = new GrandSearchListModel(3, 3, this);
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
        QStandardItem *newItem = new QStandardItem;
        m_model->appendRow(newItem);

        auto row = m_model->rowCount() - 1;
        QModelIndex index = m_model->index(row, 0, QModelIndex());

        QVariant searchMeta;
        searchMeta.setValue(item);
        m_model->setData(index, searchMeta, DATA_ROLE);

        // 设置icon
        QVariant IconMeta;
        QString imagesDirPath = item.icon;
        //QString imagesDirPath = cacheDir() + "/images/" + result.icon + ".jpg";
        QFileInfo file(imagesDirPath);
        if (file.exists()) {
            QPixmap icon(imagesDirPath);
            IconMeta.setValue(icon);
            m_model->setData(index, IconMeta, ICON_ROLE);
        } else {
            IconMeta.setValue(m_defaultIcon);
            m_model->setData(index, IconMeta, ICON_ROLE);
        }
    }
}

int GrandSearchListview::rowCount()
{
    qDebug() << QString("GrandSearchListview::rowCount: %1").arg(m_model->rowCount());
    return m_model->rowCount();
}

int GrandSearchListview::getIndexInt()const
{
    return m_CurrentIndex;
}

void GrandSearchListview::setCurrentIndexInt(int row)
{
    m_CurrentIndex = row;
}

int GrandSearchListview::getThemeType() const
{
    return m_themeType;
}

void GrandSearchListview::onItemClicked(const QModelIndex &index)
{
    qDebug() << QString("GrandSearchListView::onItemClicked item clicked r[%1] c[%2]").arg(index.row()).arg(index.column());
}

void GrandSearchListview::onSetThemeType(int type)
{
    m_themeType = type;
}

void GrandSearchListview::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "GrandSearchListview::mouseMoveEvent";
}

bool GrandSearchListview::event(QEvent *event)
{
    Q_UNUSED(event)
    return QListView::event(event);
}

QString GrandSearchListview::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}
