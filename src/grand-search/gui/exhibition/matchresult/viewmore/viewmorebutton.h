/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef VIEWMOREBUTTON_H
#define VIEWMOREBUTTON_H

#include <QToolButton>

class ViewMoreButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ViewMoreButton(const QString &text, QWidget *parent = nullptr);
    void setSelected(bool selected);
    bool isSelected();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    bool m_bIsSelected = false;
};

#endif // VIEWMOREBUTTON_H
