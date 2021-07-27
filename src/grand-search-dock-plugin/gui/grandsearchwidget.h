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
#ifndef GRANDSEARCHWIDGET_H
#define GRANDSEARCHWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPixmap>

class GrandSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrandSearchWidget(QWidget *parent = nullptr);
    ~GrandSearchWidget() override;

    void updateIcon();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    QIcon m_icon;
    QPixmap m_pixmap;
};

#endif // GRANDSEARCHWIDGET_H
