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
#ifndef ENTRANCEWIDGET_P_H
#define ENTRANCEWIDGET_P_H

#include "QObject"
#include "entrancewidget.h"

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
DWIDGET_END_NAMESPACE

class QHBoxLayout;
class QSpacerItem;
class QTimer;
class EntranceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    void delayChangeText();
    void notifyTextChanged();

    EntranceWidget *q_p;
    Dtk::Widget::DSearchEdit *m_searchEdit = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;
    QSpacerItem *m_horSpace = nullptr;//搜索框后的小空隙，用于展示界面垂直滚动条对齐

    QTimer *m_delayChangeTimer = nullptr;      // 延迟发出搜索文本改变
};

#endif // ENTRANCEWIDGET_P_H
