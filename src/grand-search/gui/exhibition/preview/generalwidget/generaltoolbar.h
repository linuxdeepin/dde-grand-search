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
#ifndef GENERALTOOLBAR_H
#define GENERALTOOLBAR_H

#include <DWidget>
#include <QToolButton>

DWIDGET_BEGIN_NAMESPACE
class DVerticalLine;
DWIDGET_END_NAMESPACE
class QHBoxLayout;

namespace GrandSearch {

class IconButton: public QToolButton
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);
};

class GeneralToolBar : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit GeneralToolBar(QWidget *parent = nullptr);

signals:
    void sigOpenClicked();
    void sigOpenPathClicked();
    void sigCopyPathClicked();

private:
    void initUi();
    void initConnect();

private:
    QHBoxLayout* m_hMainLayout = nullptr;

    IconButton *m_openBtn = nullptr;
    IconButton *m_openPathBtn = nullptr;
    IconButton *m_copyPathBtn = nullptr;

    Dtk::Widget::DVerticalLine* m_vLine1 = nullptr;
    Dtk::Widget::DVerticalLine* m_vLine2 = nullptr;
};

}

#endif // GENERALTOOLBAR_H
