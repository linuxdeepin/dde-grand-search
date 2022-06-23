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

#include "global/matcheditem.h"
#include "gui/datadefine.h"

#include <DListView>

#include <QPixmap>

#define ICON_ROLE Qt::UserRole+1
#define DATA_ROLE Qt::UserRole+2

namespace GrandSearch {

class GrandSearchListModel;
class GrandSearchListDelegate;
class GrandSearchListView: public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    explicit GrandSearchListView(QWidget *parent = Q_NULLPTR);
    ~GrandSearchListView() override;

    void setMatchedItems(const MatchedItems &items);
    void addRow(const MatchedItem &item);
    void addRow(const MatchedItem &item, const int level);
    void addRows(const MatchedItems &items);
    void addRows(const MatchedItems &items, const int level);
    int insertRow(int nRow, const MatchedItem &item);
    void insertRows(int nRow, const MatchedItems &items);
    // 从给定行开始删除指定行数
    void removeRows(int nRow, int nCount);

    // 对应level级别项的数量,默认返回所有层级项的总和
    int levelItemCount(const int level = -1);

    int rowCount();
    int getThemeType() const;

    void clear();

    void updatePreviewItemState(const bool preview);
    bool isPreviewItem() const;

public slots:
    void onSetThemeType(int type);

signals:
    void sigCurrentItemChanged(const MatchedItem &item);
    void sigItemClicked();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QString cacheDir();
    void setData(const QModelIndex& index, const MatchedItem &item);
    int levelItemLastRow(const int level);

private:
    GrandSearchListModel        *m_model        = nullptr;
    GrandSearchListDelegate     *m_delegate     = nullptr;

    int                         m_themeType     = 1;// 当前应用主题类型 1:浅色 2:深色 默认1:浅色
    MatchedItems                m_matchedItems;

    bool                        m_isPreviewItem = false;
};

}

#endif // GRANDSEARCHLISTVIEW_H
