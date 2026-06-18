// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMARTSEARCHWIDGET_H
#define SMARTSEARCHWIDGET_H

#include <DWidget>
#include <DSwitchButton>

namespace GrandSearch {

class TipsLabel;

class SmartSearchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit SmartSearchWidget(QWidget *parent = nullptr);
    ~SmartSearchWidget() override;

private slots:
    void onSwitchToggled(bool checked);

private:
    void initUi();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DSwitchButton *m_switchBtn = nullptr;
};

}

#endif // SMARTSEARCHWIDGET_H
