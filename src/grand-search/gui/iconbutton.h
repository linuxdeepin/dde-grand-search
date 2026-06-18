// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <DIconButton>

namespace GrandSearch {

class IconButton : public Dtk::Widget::DIconButton
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}

#endif // ICONBUTTON_H
