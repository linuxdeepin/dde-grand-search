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

class MatchWidgetPrivate;
class QVBoxLayout;
class MatchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit MatchWidget(QWidget *parent = nullptr);
    ~MatchWidget();

    void reLayout();
    void setMatchedData(const MatchedItemMap &matchedData);

public slots:


protected:
    void initUi();
    void initConnect();

    void paintEvent(QPaintEvent *event) override;
private:
    QScopedPointer<MatchWidgetPrivate> d_p;

    QVBoxLayout *m_vLayout = nullptr;
    GroupWidgetMap m_GroupWidgetMap;// 按group哈希值存放组列表

    MatchedItemMap m_MatchedItemMap;// 匹配结果数据

    // 依据产品需求，规定group哈希值对应组列表显示顺序
    QStringList m_GroupHashShowOrder{
        AC_GroupHash_App,
        AC_GroupHash_Folder,
        AC_GroupHash_File};

    GroupWidgets m_vGroupWidgets;// 组列表按显示顺序存放
};

#endif // MATCHWIDGET_H
