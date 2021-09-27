/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
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
#ifndef MATCHWIDGET_H
#define MATCHWIDGET_H

#include "groupwidget.h"

#include <DWidget>

#include <QScopedPointer>

class GroupWidget;
class MatchWidgetPrivate;
class QVBoxLayout;
class MatchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit MatchWidget(QWidget *parent = nullptr);
    ~MatchWidget() override;

public slots:
    void appendMatchedData(const GrandSearch::MatchedItemMap &matchedData);//追加显示匹配数据
    void clearMatchedData();
    void onSearchCompleted();
    void onShowMore();

    void selectNextItem();
    void selectPreviousItem();
    void handleItem();

private slots:
    void onSelectItemByMouse(const GrandSearch::MatchedItem &item);

signals:
    void sigCurrentItemChanged(const QString &searchGroupName, const GrandSearch::MatchedItem &item);
    void sigShowNoContent(bool noContent);
    void sigCloseWindow();

private:
    // 在显示列表中，选择指定组的第一个项
    // 组或组的视图为空，或组的视图中项为空，则查找下一个组
    // 从给定的组查找到最后仍然失败，则返回false，否则返回true
    bool selectFirstItem(int groupNumber = 0);

    // 选择给定组的最后一项
    bool selectLastItem(int groupNumber);

    bool hasSelectItem();
    bool hasSelectItem(int groupNumber);

    void clearSelectItem();

    void adjustScrollBar();

    void currentIndexChanged(const QString &searchGroupName, const QModelIndex& index);

protected:
    void initUi();
    void initConnect();
    void reLayout();

    // 动态创建类目列表，若对应类目已存在，则返回已有类目列表
    GroupWidget *createGroupWidget(const QString &searchGroupName);

    // 对要显示的类目列表进行排序
    void sortVislibleGroupList();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QScopedPointer<MatchWidgetPrivate> d_p;

    QVBoxLayout *m_vMainLayout = nullptr;               // 匹配界面主体布局
    Dtk::Widget::DScrollArea *m_scrollArea = nullptr;   // 滚动区域部件
    DWidget *m_scrollAreaContent = nullptr;             // 滚动区域内容部件
    QVBoxLayout *m_vScrollLayout = nullptr;             // 滚动区域内部部件整体布局

    QStringList m_groupHashShowOrder;                   // 类目列表显示顺序

    GroupWidgetMap m_groupWidgetMap;                    // 按group类名存放类目列表
    GroupWidgets m_vGroupWidgets;                       // 类目列表按显示顺序存放

    bool m_customSelected = false;                      // 记录用户是否手动选中
};

#endif // MATCHWIDGET_H
