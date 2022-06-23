/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#ifndef INDEXWIDGET_H
#define INDEXWIDGET_H

#include <DWidget>

#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class BlackListWidget;
class IndexWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit IndexWidget(QWidget *parent = nullptr);
    ~IndexWidget();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    BlackListWidget *m_blackListWidget = nullptr;
};

}
#endif // INDEXWIDGET_H
