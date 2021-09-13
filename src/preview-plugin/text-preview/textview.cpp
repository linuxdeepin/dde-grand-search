/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "textview.h"

#include <QTextBrowser>
#include <QHBoxLayout>


TextView::TextView(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    this->setLayout(layout);
    layout->setMargin(0);

    view = new QTextBrowser(this);
    layout->addWidget(view);
}

void TextView::setSource(const QString &path)
{
    view->setSource(QUrl::fromLocalFile(path));
}
