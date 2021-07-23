/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include "../../datadefine.h"
#include "../../../../global/matcheditem.h"

#include <DWidget>

#include <QScopedPointer>


class GroupWidgetPrivate;
class GrandSearchListview;

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DHorizontalLine;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

//调试使用，最后发布时需删除todo
//定义显示窗口背景色宏，若使用，可显示各个界面背景色，用于更好调整窗口布局和大小，该宏仅在dev阶段使用
//#define SHOW_BACKCOLOR

class QVBoxLayout;
class QHBoxLayout;
class GroupWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GroupWidget(QWidget *parent = nullptr);
    ~GroupWidget();

    void setGroupName(const QString &groupHash);
    void appendMatchedItems(const GrandSearch::MatchedItems &items);
    void showHorLine(bool bShow = true);
    bool isHorLineVisilbe();
    GrandSearchListview *getListView();
    int itemCount();
    void reLayout();
    void clear();

    QString groupName();
    static QString getGroupName(const QString &groupHash);
    static QString getGroupObjName(const QString &groupHash);

protected:
    void initUi();
    void initConnect();

    void paintEvent(QPaintEvent *event) override;

public slots:
    void setThemeType(int type);
    void onMoreBtnClcked();

signals:
    void showMore();

private:
    QScopedPointer<GroupWidgetPrivate> d_p;

    QVBoxLayout *m_vLayout = nullptr;//主体垂直布局
    QHBoxLayout *m_hTitelLayout = nullptr;//顶部标题水平布局

    DLabel *m_groupLabel = nullptr;
    DPushButton* m_viewMoreButton = nullptr;
    GrandSearchListview *m_listView = nullptr;
    DHorizontalLine *m_line = nullptr;

    bool m_bListExpanded = false;                   // 结果列表是否已展开
    GrandSearch::MatchedItems m_firstFiveItems;     // 前5行正在显示的匹配结果
    GrandSearch::MatchedItems m_restShowItems;      // 剩余正在显示的匹配结果
    GrandSearch::MatchedItems m_cacheItems;         // 缓存中的匹配结果
};

typedef QMap<QString, GroupWidget *> GroupWidgetMap;
typedef QList<GroupWidget *> GroupWidgets;

#endif // GROUPWIDGET_H
