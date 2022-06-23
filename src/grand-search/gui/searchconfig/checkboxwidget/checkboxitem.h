/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include <DWidget>
#include <DCheckBox>

namespace GrandSearch {

class CheckBoxItem : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CheckBoxItem(const QString &text, QWidget *parent = nullptr);
    ~CheckBoxItem();

    void setChecked(bool checked);
    void setTopRound(bool round);
    void setBottomRound(bool round);
    QString text() const;

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Dtk::Widget::DCheckBox *m_checkBox = nullptr;
    bool m_topRound = false;
    bool m_bottomRound = false;
};

}

#endif   // CHECKBOXITEM_H
