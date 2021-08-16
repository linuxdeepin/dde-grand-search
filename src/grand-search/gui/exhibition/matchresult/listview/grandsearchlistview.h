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
#ifndef GRANDSEARCHLISTVIEW_H
#define GRANDSEARCHLISTVIEW_H

#include "../../../../../global/matcheditem.h"
#include "../../../datadefine.h"
using namespace GrandSearch;

#include <DListView>

#include <QPixmap>

DWIDGET_USE_NAMESPACE

#define ICON_ROLE Qt::UserRole+1
#define DATA_ROLE Qt::UserRole+2
#define GROUP_ROLE Qt::UserRole+3

class GrandSearchListModel;
class GrandSearchListDelegate;
class GrandSearchListview: public DListView
{
    Q_OBJECT
public:
    explicit GrandSearchListview(QWidget *parent = Q_NULLPTR);
    ~GrandSearchListview() override;

    void setMatchedItems(const MatchedItems &items, const QString &group);
    void addRow(const MatchedItem &item, const QString &group);
    void addRows(const MatchedItems &items, const QString &group);
    int insertRow(int nRow, const MatchedItem &item, const QString &group);
    void insertRows(int nRow, const MatchedItems &items, const QString &group);
    // 从给定行开始删除指定行数
    void removeRows(int nRow, int nCount);

    // 获取小组类型下所有匹配结果
    MatchedItems groupItems(const QString& group);

    // 获取指定小组类型下最后显示的行号
    int lastShowRow(const QString &group);

    int rowCount();
    int getThemeType() const;

    void clear();

public slots:
    void onItemClicked(const QModelIndex &index);
    void onSetThemeType(int type);

signals:
    void sigAppIconChanged(const QString &appIconName);
    void sigSelectItemByMouse(GrandSearchListview *listView);
    void sigItemClicked();

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool viewportEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    QString cacheDir();
    void setData(const QModelIndex& index, const MatchedItem &item, const QString &group);

private:
    GrandSearchListModel        *m_model        = nullptr;
    GrandSearchListDelegate     *m_delegate     = nullptr;

    int                         m_themeType     = 1;// 当前应用主题类型 1:浅色 2:深色 默认1:浅色
    MatchedItems                m_matchedItems;
};

#endif // GRANDSEARCHLISTVIEW_H
