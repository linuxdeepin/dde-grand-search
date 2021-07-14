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

class GrandSearchListModel;
class GrandSearchListDelegate;
class GrandSearchListview: public DListView
{
    Q_OBJECT
public:
    explicit GrandSearchListview(QWidget *parent = Q_NULLPTR);
    ~GrandSearchListview() override;

    void setMatchedItems(const MatchedItems &items);
    int rowCount();
    int getIndexInt()const;
    void setCurrentIndexInt(int row);
    int getThemeType() const;

public slots:
    void onItemClicked(const QModelIndex &index);
    void onSetThemeType(int type);
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *event)Q_DECL_OVERRIDE;

private:
    QString cacheDir();

private:
    GrandSearchListModel        *m_model        = nullptr;
    GrandSearchListDelegate     *m_delegate     = nullptr;

    int                         m_themeType     = 1;// 当前应用主题类型 1:浅色 2:深色 默认1:浅色
    int                         m_CurrentIndex = -1;// 键盘上下移动选择时，记录当前选择的行索引，用于绘制选中的行
    QPixmap                     m_defaultIcon = QPixmap(":/icons/deepin/builtin/actions/deepin_file.svg");

    MatchedItems                m_matchedItems;
};

#endif // GRANDSEARCHLISTVIEW_H
