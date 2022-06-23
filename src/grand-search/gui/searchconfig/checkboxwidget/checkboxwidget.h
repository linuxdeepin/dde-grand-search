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
#ifndef CHECKBOXWIDGET_H
#define CHECKBOXWIDGET_H

#include <DWidget>

#include <QVBoxLayout>

namespace GrandSearch {

class CheckBoxItem;
class CheckBoxWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CheckBoxWidget(QWidget *parent = nullptr);
    ~CheckBoxWidget();

    int addCheckBox(const QString &text, bool isChecked = false);
    void addCheckBox(int index, CheckBoxItem *item, bool isChecked = false);
    bool setCheckBoxChecked(int index, bool isChecked);
    bool setCheckBoxChecked(const QString &text, bool isChecked);

signals:
    void checkedChanged(int index, bool checked);

private slots:
    void onCheckBoxChecked(bool checked);

private:
    QList<CheckBoxItem *> m_checkBoxList;
    QVBoxLayout *m_mainLayout;
};

}

#endif   // CHECKBOXWIDGET_H
