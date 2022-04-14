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
#ifndef TAILWIDGET_H
#define TAILERWIDGET_H

#include "checkboxwidget/checkboxwidget.h"

#include <DWidget>
#include <DLabel>
#include <DCheckBox>

#include <QVBoxLayout>

class TailerWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    enum TailerProperty
    {
        ParentDirectory,
        TimeModified
    };

    explicit TailerWidget(QWidget *parent = nullptr);
    ~TailerWidget();

private slots:
    void onCheckBoxStateChanged(int index, bool checked);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    CheckBoxWidget *m_checkBoxWidget = nullptr;
};

#endif   // TAILERWIDGET_H
