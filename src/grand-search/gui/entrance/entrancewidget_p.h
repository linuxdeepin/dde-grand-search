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

#include "entrancewidget.h"

namespace Dtk {
    namespace Widget {
        class DSearchEdit;
    }
}

class QHBoxLayout;
class QTimer;

class EntranceWidgetPrivate
{
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    void delayChangeText();

    EntranceWidget *q_p;
    Dtk::Widget::DSearchEdit *m_searchEdit = nullptr;
    QHBoxLayout *m_mainLayout = nullptr;

    QTimer *m_timer = nullptr;      // 延迟发出搜索文本改变
};

#endif // ENTRANCEWIDGET_P_H
