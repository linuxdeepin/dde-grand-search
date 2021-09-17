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
class DLabel;
DWIDGET_END_NAMESPACE

class QHBoxLayout;
class QTimer;
class QLineEdit;
class QAction;
class QPushButton;
class EntranceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    void delayChangeText();
    void notifyTextChanged();
    void setLineEditFocus();

    void onFocusObjectChanged(QObject *obj);

    void showMenu(const QPoint& pos);

    EntranceWidget *q_p = nullptr;
    Dtk::Widget::DSearchEdit *m_searchEdit = nullptr;   // 搜索输入框控件
    QLineEdit *m_lineEdit = nullptr;                    // 输入控件
    Dtk::Widget::DLabel *m_appIconLabel = nullptr;      // 应用图标显示label
    QAction *m_appIconAction = nullptr;                 // 应用图标显示区域占位action
    QHBoxLayout *m_mainLayout = nullptr;

    QTimer *m_delayChangeTimer = nullptr;               // 延迟发出搜索文本改变

    QString m_appIconName;                              // 当前搜索框显示的默认打开应用图标名称
};

#endif // ENTRANCEWIDGET_P_H
